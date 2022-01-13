%% CONCEPT TEST - LINE ALGORITHM DEEP SECTION USEFUL ONLY. UPGRADE OF V2
clear all;
clc;

data = readtable('surfaces_xl/1.csv');
data = data{:,:};
disp('imported');

%% SURFACE
d = data;
d2 = data(1:2:end, 1:2:end);

s = size(d);
[x,y] = meshgrid(1:s(1), 1:s(2));
surf(x', y', d);
hold on;
[x,y] = meshgrid(1:2:s(1), 1:2:s(2));
surf(x', y', d2);
hold off;

%% LINE ALGORITHM DEEP
clc;
format long g
a = data(1:30, 2);
p = length(a);
x = 1:p;

ai = floor(linspace(1, p, 5));
ti = ai;
figure;
change_factor = 0.01;
next_index_ratio = 0.5;
while ~isempty(ai)
    disp(ai);
    j = j + 1;
    plot(x, a, '.-b');
    hold on;
    ai = next_indices(a, ai, x, change_factor, next_index_ratio);
    ti = [ti, ai];
    ylim([min(a) * 0.5, max(a) * 1.05]);
    hold off;
    grid on;
    grid minor;
    if ~isempty(ai)
        pause;
    end
end

ti = unique(ti);
total_iterations = length(ti);
efficiency = total_iterations / p;

fprintf("its: %i, eff: %.1f \n", total_iterations, efficiency*100);

plot(x, a, '.-r');
hold on;
plot(x(ti), a(ti), 'og');
ylim([min(a) * 0.5, max(a) * 1.05]);
hold off;
grid on;
grid minor;
pause(1);

%% LINE ALGORITH SHORT
clc;
format long g
figure;

 change_factor = 0.01;
 next_index_ratio = 0.5;
 num_correct = 0;
 
 minima = zeros(1, size(data, 2));
 sum_its = 0;
for r = 1:size(data, 2)
    a = data(1:30, r);
    [~, ind] = min(a);
    p = length(a);
    x = 1:p;
    ai = floor(linspace(1, p, 5));
    ti = ai;
    while ~isempty(ai)
        plot(x, a, '.-b');
        ai = next_indices(a, ai, x, change_factor, next_index_ratio);
        ti = [ti, ai];
    end

    ti = unique(ti);
    total_iterations = length(ti);
    efficiency = total_iterations / p;
    sum_its = sum_its + total_iterations;
    if any(ti == ind)
        fprintf("its: %i, eff: %.1f, correct!\n", total_iterations, efficiency*100);
        num_correct = num_correct + 1;
    else
        fprintf("its: %i, eff: %.1f \n", total_iterations, efficiency*100);
    end
    minima(r) = min(a);
    plot(x, a, '.-r');
    hold on;
    plot(x(ti), a(ti), 'og');
    ylim([min(a) * 0.5, max(a) * 1.05]);
    hold off;
    grid on;
    grid minor;
    %pause;
end

plot(minima)

fprintf("correct: %i, reliability: %.1f \n", num_correct, num_correct/size(data, 2) * 100);

sum_its / length(data(:))

%%
function ii = next_indices(a, ai, x, change_factor, next_index_ratio)
    %plot(x(ai), a(ai), '.-r');
    [min_val, ~] = min(a(ai));
%     change_factor = 0.5;
%     next_index_ratio = 0.5;
    ii = [];
    for i = 1:length(ai) - 1
        c1 = a(ai(i));
        c2 = a(ai(i+1));
        
        if ai(i+1) - ai(i) > 1
            m = (c2 - c1) * change_factor;

            if m > 0
                estimate = c1 - m;
            else
                estimate = c2 + m;
            end

            if estimate <= min_val
                if m > 0
                    mid_index = floor(ai(i) + next_index_ratio * (ai(i+1) - ai(i)));
                else
                    mid_index = floor(ai(i+1) - next_index_ratio * (ai(i+1) - ai(i)));
                end
                if mid_index == ai(i)
                    mid_index = ai(i) + 1;
                elseif mid_index == ai(i+1)
                    mid_index = ai(i+1) - 1;
                end
                ii = [ii, ai(i), mid_index, ai(i+1)];
                x_mid = floor(next_index_ratio * (x(ai(i)) + x(ai(i + 1))));
                plot([x(ai(i)), x(ai(i + 1))], [c1, c2], '.-g', 'linewidth', 2, 'MarkerSize', 20);
                plot(x_mid, estimate, '*g');
            else
                x_mid = floor(next_index_ratio * (x(ai(i)) + x(ai(i + 1))));
                plot([x(ai(i)), x(ai(i + 1))], [c1, c2], '.-r', 'linewidth', 2, 'MarkerSize', 20);
                plot(x_mid, estimate, '*r');
                %plot(floor(next_index_ratio * (x(ai(i)) + x(ai(i + 1)))), estimate, '.c');
            end
        end
    end
    ii = unique(ii);
end


function nextIndices()
end