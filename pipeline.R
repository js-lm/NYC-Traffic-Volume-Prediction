install.packages("recipes")
install.packages("doParallel")
install.packages("progressr")
install.packages('rsvg')
install.packages("DiagrammeRsvg")

# Load Packages and Data 
library(tidyverse)
library(lubridate)
library(caret)
library(xgboost)
library(dplyr)
library(rpart.plot)
library(DiagrammeR)
library(recipes)
library(tidymodels)
library(rsvg)
library(DiagrammeRsvg)
library(here)

library(progressr)
handlers(global = TRUE)
handlers("txtprogressbar") 

library(doParallel)
# run parallel to make tuning faster
cl <- makePSOCKcluster(parallel::detectCores())
registerDoParallel(cl)

# load and preprocess data 
data <- read.csv(here("final_merged_dataset.csv")) %>%
  filter(
    str_detect(street, regex(
      "BELT|BELT PKWY|SHORE PKWY|SHORE|BQE|BROOKLYN QUEENS|CROSS BRONX|DEEGAN",
      ignore_case = TRUE
    ))
  ) %>%
  mutate(
    road_group = case_when(
      # Belt Parkway + Shore Parkway
      str_detect(street, regex("BELT|BELT PKWY|SHORE", ignore_case = TRUE)) ~ "Belt Parkway",
      
      # BQE
      str_detect(street, regex("BQE|BROOKLYN QUEENS", ignore_case = TRUE)) ~ "BQE",
      
      # Cross Bronx
      str_detect(street, regex("CROSS BRONX", ignore_case = TRUE)) ~ "Cross Bronx Expressway",
      
      # Major Deegan
      str_detect(street, regex("DEEGAN", ignore_case = TRUE)) ~ "Major Deegan",
      
      TRUE ~ "Other"
    )
  ) %>%
  mutate(time = ymd_hm(time)) %>%
  select(-street)

data <- data %>%
  rename(
    Temp = temperature_2m...C.,
    RH = relative_humidity_2m....,
    Dew_Point = dew_point_2m...C.,
    Precip_mm = precipitation..mm.,
    Temp_Apparent = apparent_temperature...C.,
    Rain_mm = rain..mm.,
    Snow_Depth_m = snow_depth..m.,
    Snowfall_cm = snowfall..cm.,
    Cloud_Cov_High = cloud_cover_high....,
    Cloud_Cov_Mid = cloud_cover_mid....,
    Cloud_Cov_Low = cloud_cover_low....,
    Cloud_Cover = cloud_cover....,
    Wind_Gusts = wind_gusts_10m..km.h.,
    Wind_Dir_10m = wind_direction_10m....,
    Wind_Dir_100m = wind_direction_100m....,
    Wind_Speed_10m = wind_speed_10m..km.h.,
    Wind_Speed_100m = wind_speed_100m..km.h.
  )


train_index <- createDataPartition(data$road_group, p = 0.8, list = FALSE)
traindata <- data[train_index, ]
testdata  <- data[-train_index, ]

rec_fixed <- rec %>%
  update_role(all_of("volume"), new_role = "outcome") %>%
  step_novel(all_nominal_predictors()) %>%
  step_dummy(all_nominal_predictors())

# Prep the recipe on training data
rec_fixed <- prep(rec_fixed, training = traindata)

# Bake training and test data
X_train <- bake(rec_fixed, new_data = traindata) %>% select(-volume)
X_test  <- bake(rec_fixed, new_data = testdata) %>% select(-volume)

# Define XGBoost Model
xgb_spec = boost_tree(
  trees = 150,
  tree_depth= tune(),
  min_n = tune(),
  learn_rate=tune(),
  loss_reduction = tune(),
  mtry = tune()
) %>%
  set_mode("regression") %>%
  set_engine("xgboost")

# Workflow
wf = workflow() %>%
  add_recipe(rec) %>%
  add_model(xgb_spec)

params <- parameters(
  tree_depth(range = c(3L, 6L)),
  min_n(range = c(5L, 15L)),
  learn_rate(range = c(0.01, 0.1)),
  loss_reduction(range = c(0.001, 1)),
  finalize(mtry(), traindata)
)

# Cross validation split
cv_splits <- vfold_cv(traindata, v = 5)

# hyperparameter grid
grid <- grid_space_filling( params ,
  size = 20
)

ctrl <- control_grid(
  save_pred = TRUE,
  verbose = TRUE,
  allow_par = TRUE
)

# hyperparameter tuning
tune_res <- tune_grid(
  wf,
  resamples = cv_splits,
  grid = grid,
  metrics = metric_set(rmse),
  control = ctrl
)

best <- select_best(tune_res, metric = "rmse")
autoplot(tune_res)
show_best(tune_res, metric = "rmse")

final_wf <- wf %>%
  finalize_workflow(best)

# fit final model
final_fit = (fit(final_wf, data = traindata))

# predictions 
traindata$pred_primary <- predict(final_fit, traindata)$.pred
testdata$pred_primary <- predict(final_fit, testdata)$.pred

# Residuals
traindata$residual <- traindata$volume - traindata$pred_primary
testdata$residual <- testdata$volume - testdata$pred_primary

# RMSE
rmse_train <- sqrt(mean((traindata$volume - traindata$pred_primary)^2))
rmse_test <- sqrt(mean((testdata$volume - testdata$pred_primary)^2))

cat("Primary Model Train RMSE:", rmse_train, "\n")
cat("Primary Model Test RMSE:", rmse_test, "\n")




# 2. Train secondary models by road group
road_groups <- unique(traindata$road_group)
secondary_models <- list()

# Ensure residual column exists
traindata <- traindata %>% mutate(residual = volume - pred_primary)

for (rg in road_groups) {
  cat("Training secondary model for", rg, "\n")
  
  # Subset training data for this road group
  train_rg <- traindata %>% filter(road_group == rg)

  
  
  # Prepare predictors for residual model
  predictor_cols <- train_rg %>% select(-volume, -pred_primary, -road_group, -time)
  n_predictors <- ncol(predictor_cols)
  
  # Dynamic mtry range: 50%-100% of predictors, minimum 2
  mtry_min <- max(2, floor(0.5 * n_predictors))
  mtry_max <- n_predictors
  mmtry_grid <- seq(mtry_min, mtry_max, by = 2)
  
  
  # Build residual recipe
  rec_resid <- recipe(residual ~ ., data = train_rg) %>%
    step_rm(volume, pred_primary, road_group, time) %>%
    step_dummy(all_nominal_predictors())
  
  # Define hyperparameter grid per road group
  grid_rg <- expand_grid(
    tree_depth = 2:4,
    min_n = c(15, 20),
    learn_rate = c(0.005, 0.01, 0.02),
    loss_reduction = c(0.01, 0.05, 0.1),
    mtry = mtry_grid
  )
  
  # XGBoost model with tunable parameters
  xgb_resid <- boost_tree(
    trees = 30,
    tree_depth = tune(),
    min_n = tune(),
    learn_rate = tune(),
    loss_reduction = tune(),
    mtry = tune()
  ) %>%
    set_mode("regression") %>%
    set_engine(
      "xgboost",
      nthread = parallel::detectCores(),
      early_stopping_rounds = 15,
      eval_metric = "rmse"
    )
  
  # Workflow
  wf_resid <- workflow() %>%
    add_recipe(rec_resid) %>%
    add_model(xgb_resid)
  
  # Cross-validation
  cv_splits_rg <- vfold_cv(train_rg, v = 3)
  
  # Tune grid
  tune_res_rg <- tune_grid(
    wf_resid,
    resamples = cv_splits_rg,
    grid = grid_rg,
    metrics = metric_set(rmse),
    control = control_grid(save_pred = TRUE)
  )
  
  # Select best hyperparameters
  best_rg <- select_best(tune_res_rg, metric = "rmse")
  cat("Best hyperparameters for", rg, ":\n")
  print(best_rg)
  
  # Finalize workflow with best params
  final_wf_resid <- wf_resid %>% finalize_workflow(best_rg)
  
  # Fit final model
  fit_resid <- fit(final_wf_resid, data = train_rg)
  
  # Store model in list and save to disk
  secondary_models[[rg]] <- fit_resid
  saveRDS(fit_resid, paste0("secondary_model_", gsub(" ", "_", rg), ".rds"))
  
  cat("Saved model for", rg, "to disk.\n\n")
}

# 3. Make hierarchical predictions
testdata$pred_hierarchical <- testdata$pred_primary

for (rg in road_groups) {
  # Subset test data for road group
  test_rg <- testdata %>% filter(road_group == rg)
  if (nrow(test_rg) == 0) next
  
  # Predict residuals for this group
  pred_resid <- predict(secondary_models[[rg]], test_rg)$.pred
  
  # Add residual prediction to primary prediction
  testdata$pred_hierarchical[testdata$road_group == rg] <- 
    testdata$pred_hierarchical[testdata$road_group == rg] + pred_resid
}

# 4. Compute RMSE

# Initialize predictions
pred_hier_train <- traindata$pred_primary
pred_hier_test  <- testdata$pred_primary

# Apply secondary models
for (rg in names(secondary_models)) {
  idx_train <- which(traindata$road_group == rg)
  idx_test  <- which(testdata$road_group == rg)
  
  if(length(idx_train) > 0){
    pred_hier_train[idx_train] <- pred_hier_train[idx_train] + 
      predict(secondary_models[[rg]], traindata[idx_train, ])$.pred
  }
  
  if(length(idx_test) > 0){
    pred_hier_test[idx_test] <- pred_hier_test[idx_test] + 
      predict(secondary_models[[rg]], testdata[idx_test, ])$.pred
  }
}

# Overall RMSE
rmse_hier_train <- sqrt(mean((traindata$volume - pred_hier_train)^2))
rmse_hier_test  <- sqrt(mean((testdata$volume - pred_hier_test)^2))

cat("Hierarchical Model Train RMSE:", rmse_hier_train, "\n")
cat("Hierarchical Model Test RMSE:", rmse_hier_test, "\n\n")

# RMSE by road group - TRAIN
cat("Train RMSE by Road Group:\n")
train_rmse_by_rg <- traindata %>%
  mutate(pred_hier = pred_hier_train) %>%
  group_by(road_group) %>%
  summarise(RMSE = sqrt(mean((volume - pred_hier)^2)))
print(train_rmse_by_rg)

# RMSE by road group - TEST
cat("\nTest RMSE by Road Group:\n")
test_rmse_by_rg <- testdata %>%
  mutate(pred_hier = pred_hier_test) %>%
  group_by(road_group) %>%
  summarise(RMSE = sqrt(mean((volume - pred_hier)^2)))
print(test_rmse_by_rg)





#visual
rec_final <- prep(rec, training = traindata)
X_train <- bake(rec_final, new_data = traindata) %>% select(-volume)  # remove target

# Extract xgboost model
xgb_model <- extract_fit_parsnip(final_fit)$fit

# Use the processed column names for plotting
feature_names <- colnames(X_train)

# Plot first tree, top-down
graph <- xgb.plot.tree(
  feature_names = feature_names,
  model = xgb_model,
  trees = 0,
  show_node_id = TRUE,
  direction = "TD"
)

# Export as SVG
svg <- export_svg(graph)

# Convert SVG to high-res PNG
rsvg_png(charToRaw(svg), "xgb_tree.png", width = 2000, height = 1200)

for (rg in names(secondary_models)) {
  cat("Exporting tree for road group:", rg, "\n")
  
  # Extract raw booster
  xgb_model_rg <- extract_fit_engine(secondary_models[[rg]])
  
  # Plot first tree
  graph <- xgb.plot.tree(
    model = xgb_model_rg,
    trees = 0
  )
  
  # Export SVG and convert to PNG
  svg <- DiagrammeRsvg::export_svg(graph)
  file_name <- paste0("xgb_tree_", gsub(" ", "_", rg), ".png")
  rsvg::rsvg_png(charToRaw(svg), file_name, width = 2000, height = 1200)
  
  cat("Saved PNG for", rg, "as", file_name, "\n")
}







# error analysis

# residual analysis
testdata$residuals = testdata$volume - pred_test

ggplot(testdata, aes(x = residuals)) +
  geom_histogram(bins = 50, fill="skyblue", color="black") +
  coord_cartesian(xlim = c(-500, 500))

residuals_df <- data.frame(residuals = testdata$volume - pred_test,
                           road_group = testdata$road_group)

hist_plot = ggplot(residuals_df, aes(x = residuals)) +
  geom_histogram(bins = 50, fill="skyblue", color="black") +
  stat_bin(
    bins = 50,
    aes(label = ..count..), 
    geom = "text", 
    vjust = -0.5, 
    size = 3
  ) +
  labs( x = "Residuals", y = "Count") +
  theme_minimal()

ggsave("residual_hist.png", plot = hist_plot, width = 6, height = 4, dpi = 300)

road_plot = ggplot(residuals_df, aes(x = road_group, y = residuals)) +
  geom_boxplot(fill="skyblue") +
  geom_hline(yintercept = 0, color="red", linetype="dashed") +
  labs( x="Road Group", y="Residuals") +
  theme_minimal()

ggsave("resroad.png", plot = road_plot, width = 6, height = 4, dpi = 300)

# Residuals
residuals <- testdata$volume - pred_test
testdata$residuals <- testdata$volume - pred_test

# Function to calculate counts and percentages within SDs
residuals_summary <- function(residuals) {
  sd_res <- sd(residuals)
  n <- length(residuals)
  
  within_1sd <- sum(abs(residuals) <= sd_res)
  within_2sd <- sum(abs(residuals) <= 2*sd_res)
  within_3sd <- sum(abs(residuals) <= 3*sd_res)
  
  data.frame(
    SD = c("1 SD", "2 SD", "3 SD"),
    Count = c(within_1sd, within_2sd, within_3sd),
    Percent = c(within_1sd/n*100, within_2sd/n*100, within_3sd/n*100)
  )
}


# Apply by road_group
roadgroup_residuals <- testdata %>%
  group_by(road_group) %>%
  summarise(summary = list(residuals_summary(residuals))) %>%
  tidyr::unnest(summary)

print(roadgroup_residuals)

weekend_residuals <- testdata %>%
  group_by(is_weekend) %>%
  summarise(summary = list(residuals_summary(residuals))) %>%
  tidyr::unnest(summary)

print(weekend_residuals)

testdata <- testdata %>%
  mutate(residuals = volume - pred_test,
         is_weekend = factor(is_weekend, labels = c("Weekday", "Weekend")))

# Histogram of residuals by weekend/weekday
ggplot(testdata, aes(x = residuals, fill = is_weekend)) +
  geom_histogram(position = "identity", alpha = 0.5, bins = 50) +
  geom_vline(xintercept = 0, linetype = "dashed", color = "red") +
  labs(x = "Residuals",
       y = "Count",
       fill = "Day Type") +
  theme_minimal()