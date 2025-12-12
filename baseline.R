library(tidyverse)
library(lubridate)
library(caret)
library(xgboost)
library(dplyr)
library(rpart.plot)
library(DiagrammeR)
library(recipes)
library(tidymodels)
library(here)

data <- read.csv(here("final_merged_dataset.csv")) %>%
  filter(str_detect(street, regex(
    "BELT|BELT PKWY|SHORE PKWY|BQE|BROOKLYN QUEENS EXPRESSWAY|CROSS BRONX|DEEGAN",
    ignore_case = TRUE
  ))) %>%
  mutate(
    road_group = case_when(
      str_detect(street, regex("BELT|SHORE", ignore_case = TRUE)) ~ "Belt Parkway",
      str_detect(street, regex("BQE|BROOKLYN QUEENS", ignore_case = TRUE)) ~ "BQE",
      str_detect(street, regex("CROSS BRONX", ignore_case = TRUE)) ~ "Cross Bronx Expressway",
      str_detect(street, regex("DEEGAN", ignore_case = TRUE)) ~ "Major Deegan",
      TRUE ~ "Other"
    )
  ) %>%
  mutate(time = ymd_hm(time)) %>%
  select(-street)

summary(data)

# One-hot encode the road_group and other categorical variables
dummy = dummyVars(" ~ .", data = data)
data_encoded <- data.frame(predict(dummy, newdata = data))

# Train/test split
set.seed(123)
train_index = sample(seq_len(nrow(data_encoded)), size = floor(0.8 * nrow(data_encoded)))
traindata = data_encoded[train_index, ]
testdata = data_encoded[-train_index, ]

# Separate features and target
train_x = as.matrix(traindata %>% select(-volume))
train_y = traindata$volume
test_x = as.matrix(testdata %>% select(-volume))
test_y = testdata$volume

# Baseline XGBoost (1 tree)
xgb_baseline = xgboost(
  data = train_x,
  label = train_y,
  nrounds = 1,          # single tree baseline
  objective = "reg:squarederror",
  verbose = 0
)

# Predictions
pred_train = predict(xgb_baseline, newdata = train_x)
pred_test = predict(xgb_baseline, newdata = test_x)

# RMSE
rmse_train = sqrt(mean((train_y - pred_train)^2))
rmse_test = sqrt(mean((test_y - pred_test)^2))

cat("Baseline Train RMSE:", rmse_train, "\n")
cat("Baseline Test RMSE:", rmse_test, "\n")

# Optional: visualize the first tree
xgb.plot.tree(model = xgb_baseline, trees = 0)