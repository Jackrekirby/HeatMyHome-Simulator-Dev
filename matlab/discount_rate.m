clear all;
clc;

discount_rate = 1.035;
current_discount_rate = 1.0;
cumulative_discount_rate = 0.0;
for i = 1:20
    cumulative_discount_rate = cumulative_discount_rate + 1/current_discount_rate;
    current_discount_rate = current_discount_rate * discount_rate;
end

cumulative_discount_rate
