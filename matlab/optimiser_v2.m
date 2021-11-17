clear all;
clc;

data = readtable('surfaces/1.csv');
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
j = 0;
figure;
change_factor = 1;
next_index_ratio = 0.5;
while ~isempty(ai)
    disp(j)
    disp(ai)
    j = j + 1;
    plot(x, a, '.-b');
    hold on;
    ai = next_indices(a, ai, x, change_factor, next_index_ratio);
    ti = [ti, ai];
    ylim([min(a) * 0.5, max(a) * 1.05]);
    hold off;
    grid on;
    grid minor;
    pause;
end

ti = unique(ti);
total_iterations = length(ti)
efficiency = total_iterations / p

plot(x, a, '.-r');
hold on;
plot(x(ti), a(ti), 'og');
ylim([min(a) * 0.5, max(a) * 1.05]);
hold off;
grid on;
grid minor;
pause(1);

%%
change_factor = 1;
ii = [];
for i = 1:length(ai) - 1
    
    m = a(ai(i + 1)) - a(ai(i));
    if m > 0
        estimate = a(ai(i)) - m * change_factor;
    else
        estimate = a(ai(i + 1)) + m * change_factor;
    end
    if estimate < min_val
        ii = [ii, ai(i), floor(0.5 * (ai(i) + ai(i+1))), ai(i+1)];
        plot(floor((x(ai(i)) + x(ai(i + 1))) / 2), estimate, '*g');
    else
        plot(floor((x(ai(i)) + x(ai(i + 1))) / 2), estimate, '*r');
    end
    ii = unique(ai);
end

for i = 1:length(ii)
    a(ii(i))
end

hold off;
grid on;
grid minor;

%%
clc;
format long g
figure;

 change_factor = 0.01;
 next_index_ratio = 0.5;
for r = 1:7
    a = data(1:30, r);
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
    
    fprintf("its: %i, eff: %.1f \n", total_iterations, efficiency*100);

    plot(x, a, '.-r');
    hold on;
    plot(x(ti), a(ti), 'og');
    ylim([min(a) * 0.5, max(a) * 1.05]);
    hold off;
    grid on;
    grid minor;
    pause;
end

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

            if estimate < min_val
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
                plot([x(ai(i)), x(ai(i + 1))], [c1, c2], 'o-g', 'linewidth', 2);
                plot(x_mid, estimate, '*g');
            else
                x_mid = floor(next_index_ratio * (x(ai(i)) + x(ai(i + 1))));
                plot([x(ai(i)), x(ai(i + 1))], [c1, c2], 'o-r', 'linewidth', 2);
                plot(x_mid, estimate, '*r');
                %plot(floor(next_index_ratio * (x(ai(i)) + x(ai(i + 1)))), estimate, '.c');
            end
        end
    end
    ii = unique(ii);
end


function nextIndices()
end