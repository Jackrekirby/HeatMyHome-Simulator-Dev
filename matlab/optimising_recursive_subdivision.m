clear all;
clc;

d = readmatrix("C:\Users\Jack\Downloads\optimiser_surface.csv");

sds = 3:14;
gf = 0:0.02:0.48;

passed = reshape(d(:, 1), length(sds), []);
efficiency = reshape(d(:, 2), length(sds), []);

tiledlayout(1, 2);
nexttile;
[X,Y] = meshgrid(sds, gf);
surf(X', Y', passed);
xlabel('Start Distribution');
ylabel('Gradient Factor');
zlabel('Pass Rate');
zlim([90, 105]);

nexttile;
[X,Y] = meshgrid(sds, gf);
X = X';
Y = Y';
surf(X, Y, efficiency);
hold on;

plot3(X(:), Y(:), efficiency(:) .* (passed(:) == 100), 'ro');
hold off;
xlabel('Start Distribution');
ylabel('Gradient Factor');
zlabel('Efficiency');
%zlim([5, inf]);
return

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