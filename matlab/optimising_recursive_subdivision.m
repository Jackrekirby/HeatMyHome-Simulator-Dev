%% 3D PASS RATE - POINTS SEARCHED PLOTTER, WORKING
clear all;
clc;

d = readmatrix("optimiser_surface2.csv");

sds = 4:14;
gf = 0.01:0.02:0.49;

% sds = 3:14;
% gf = 0:0.02:0.48;

passed = reshape(d(:, 1), length(sds), []);
efficiency = reshape(d(:, 2), length(sds), []);

e2 = efficiency(:);
p2 = passed(:);
p3 = passed(:);
for i = 1:length(e2)
    if p2(i) < 100
        e2(i) = NaN;
        p3(i) = NaN;
    end
end

tl = tiledlayout(1, 2);
title(tl, {"% Pass Rate and Points Searched For Different", "Starting Distributions and Gradient Factor Combinations"})
nexttile;
[X,Y] = meshgrid(sds, gf);
X = X';
Y = Y';

surf(X, Y, passed);
hold on;
plot3(X(:), Y(:), p3, 'r.', 'MarkerSize', 20);
hold off;
xlabel('Start Distribution');
ylabel('Gradient Factor');
zlabel('% Pass Rate');
zlim([90, 105]);

nexttile;
surf(X, Y, efficiency);
hold on;
plot3(X(:), Y(:), e2, 'r.', 'MarkerSize', 20);
hold off;
xlabel('Start Distribution');
ylabel('Gradient Factor');
zlabel('% Total Points Searched');
%zlim([5, inf]);

%% 2D PLOT

b = (d(:, 1) == 100) + 1;
colors = hsv(2);
figure;
hold on;
for i = 1:size(d, 1)
    plot(d(i, 1), d(i, 2), '.', 'Color', colors(b(i), :));
end
hold off;
title("Efficiency vs Pass Rate of Optimiser");
grid on;
xlim([90, 100]);
ylabel('Efficiency %');
xlabel('Pass %')