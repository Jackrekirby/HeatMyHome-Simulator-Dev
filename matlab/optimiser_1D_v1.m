%% 1D OPTIMISER VISUALISER, WORKING
clear all;
clc;

data = readtable('surfaces_xl/1.csv');
data = data{:,:};
disp('imported');

%%

clc;
format long g
d = data(1:30, 2);
len_a = length(d);
x = 1:len_a;
ai = floor(linspace(1, len_a, 5));

cf = 1;
tai = [];
while true
    plot(x, d, '.-b');
    hold on;
    plot(x(ai), d(ai), 'ob');
    tai = [tai, ai];
    ai = next_itr(d, ai, x, cf);
    hold off;
    pause;
    if isempty(ai)
        break
    end
end

tai = unique(tai);

hold on;
plot(x(ai), d(ai), 'or');
plot(x(tai), d(tai), 'ok');
hold off;
disp('done');

function nai = next_itr(d, ai, x, cf)
    [min_val, min_ind] = min(d(ai));
    [max_val, max_ind] = max(d(ai));
    h = (max_val - min_val) / abs(ai(max_ind) - ai(min_ind));
    ai
    nai = [];
    r = 0.5;
    for i = 1:length(ai) - 1
        if ai(i+1) - ai(i) > 1
            [low_val, low_ind] = min(d(ai([i, i+1])));

            y = low_val - (h * (ai(i+1) - ai(i)) * cf);
            %midx = x(ai(i)) + 0.5 * (x(ai(i+1)) - x(ai(i)));

        %     if y <= min_val
        %         plot(midx, y, 'og');
        %     else
        %         plot(midx, y, 'or');
        %     end

            if y <= min_val
                if low_ind == 1
                    mid_index = floor(ai(i) + r * (ai(i+1) - ai(i)));
                else
                    mid_index = floor(ai(i+1) - r * (ai(i+1) - ai(i)));
                end

                if mid_index == ai(i)
                    mid_index = ai(i) + 1;
                elseif mid_index == ai(i+1)
                    mid_index = ai(i+1) - 1;
                end

                nai = [nai, ai(i), mid_index, ai(i+1)];
                x_mid = floor(r * (x(ai(i)) + x(ai(i + 1))));
                %plot([x(ai(i)), x(ai(i + 1))], [d(ai(i)), d(ai(i + 1))], '.-g', 'linewidth', 2, 'MarkerSize', 20);
                plot(x_mid, y, '*g');
            else
                x_mid = floor(r * (x(ai(i)) + x(ai(i + 1))));
                %plot([x(ai(i)), x(ai(i + 1))], [d(ai(i)), d(ai(i + 1))], '.-r', 'linewidth', 2, 'MarkerSize', 20);
                plot(x_mid, y, '*r');
            end
        end
        nai = unique(nai);
    end
end

