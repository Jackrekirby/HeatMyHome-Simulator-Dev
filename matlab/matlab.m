clear all
clc

data = readtable('output_file.txt');
data = data{:,:};
%%
clc
global iterations
global learning_rate
iterations = 0;
learning_rate = 0.2;

tariff = 3;
target = data(:, 5) == tariff;
a = reshape(data(target, 6), 30, []);
b = reshape(data(target, 2), 30, []);
c = reshape(data(target, 1), 30, []);
plot(0, 0);
hold on;
colors = hsv(7);
markers = [".", ".", "."];
j = 1;
for i = 1:length(a)
    [val, ind] = min(a(:, i));
    [val2, ind2] = predictMin(a(:, i));
    if abs(val2 - val) > 1
        fprintf("val %f, ind %d\n", val2 - val, ind2-ind);
    end
    plot(i * 30 + ind-1, val, 'dg', 'MarkerSize', 7, 'LineWidth', 2);
    plot(i * 30 + ind2-1, val2, 'or', 'MarkerSize', 7, 'LineWidth', 2);
    plot(i * 30 + [0:29], a(:, i), '.-', 'color', colors(b(1, i)+1, :), 'marker', markers(c(1, i)+1));
    j = mod(j, 6) + 1;
end
hold off;
grid on;
grid minor;
ylabel('Operational Cost £')
xlabel('Iteration (Heat Option, Solar Option, Solar Sizes)')
total_its = length(data) / 5;
xlim([0, total_its])

total_its;
iterations;
saving = iterations / total_its;

fprintf("%d / %d = %f saving\n", iterations, total_its, saving);

%%
clc
a = data;
heat_opts = max(a(:, 1))
solar_opts = max(a(:, 2))
solar_sizes = max(a(:, 3))
tes_options = max(a(:, 4))
tariffs = max(a(:, 5))

for heat_opt = 0:heat_opts
    fprintf("heat_opt: %d\n", heat_opt)
    for solar_opt = 0:solar_opts
        fprintf("  solar_opt: %d\n", solar_opt)
        for solar_size = 0:solar_sizes
            fprintf("    solar_size: %d\n", solar_size)
            for tariff = 0:tariffs
                a = data;
                a = a(a(:, 1) == heat_opt, :);
                a = a(a(:, 2) == solar_opt, :);
                a = a(a(:, 3) == solar_size, :);
                %a = a(a(:, 4) == tes_option, :);
                a = a(a(:, 5) == tariff, :);
                if ~isempty(a)
                    isLinear(a(:, 6))
                end 
            end
        end
    end
end

function isLinear(a)
    cs = sign(a(2) - a(1));
    j = 0;
    for i = 2:length(a)
        b = a(i) - a(i - 1);
        c = sign(b);
        %fprintf("%d, %d, %d, %d\n", a(i), a(i-1), b, c);
        if c * cs < 0
            %disp('sign change')
            cs = c;
            j = j + 1;
        end
    end
    %fprintf("sign changes: %d\n", j);
    if j > 1
        %a
        fprintf("      SIGN CHANGES: %d\n", j);
        %disp('sign changed more than once');
        %error('sign changed more than once')
    end
end

%%

function [val, ind] = predictMin(a)
    global iterations
    global learning_rate
    iterations = iterations + 1;
    i = 1;
    step = learning_rate * (a(i) - a(i+1));
    while i < length(a)
        iterations = iterations + 1;
        last_step = step;
        step = learning_rate * (a(i) - a(i+1));
        %fprintf("step %f\n", step);
        if last_step * step < 0
            %fprintf("%f, %f, %f\n", a(i), a(i+1), abs(a(i+1) - a(i)));
            if a(i+1) < a(i)
                val = a(i+1);
                ind = i+1;
            else
                val = a(i);
                ind = i;
            end
            return
        end
        if abs(step) < 1
            i = i + sign(step);
        else
            i = i + round(step);
        end
        
    end
    val = a(1);
    ind = 1;
end













function [val, ind] = predictMin2(a)
    global iterations
    iterations = iterations + 1;
    i = 1;
    while i < length(a)
        iterations = iterations + 1;
        if abs(a(i+1) - a(i)) < 0.5
            %fprintf("%f, %f, %f\n", a(i), a(i+1), abs(a(i+1) - a(i)));
            if a(i+1) < a(i)
                val = a(i+1);
                ind = i+1;
            else
                val = a(i);
                ind = i;
            end
            return
        end
        i = i + 1;
    end
    val = a(1);
    ind = 1;
end



















