%% 3D OPTIMISER VISUALISER, WORKING BUT INCOMPLETE
clear all;
clc;

data = readtable('surfaces_xl/1.csv');
data = data{:,:};
disp('imported');

%% ALGORITHM
clc;

d = data;
s = size(d);
[X,Y] = meshgrid(1:s(1), 1:s(2));
surf(X', Y', d);
alpha 0
hold on;

xi = floor(linspace(1, s(1), 5));
yi = floor(linspace(1, s(2), 5));
[x,y] = meshgrid(xi, yi);
plot3(x', y', d(xi, yi), '.b', 'MarkerSize', 20);

cf = 1;
ir = 0.5;

minz = min(d(xi, yi));
ps = [x(:), y(:)]
points = [];
for j = 1:length(yi)-1
    for i = 1:length(xi)-1
        points = [points; plane(d, cf, ir, xi, yi, i, j, minz)];
    end
end
%points = unique(points, 'rows')
hold off;

function [xs, ys] = plane(d, cf, ir, xi, yi, x, y, minz)
    xs = [];
    ys = [];
    p11 = d(xi(x), yi(y));
    p21 = d(xi(x+1), yi(y));
    p12 = d(xi(x), yi(y+1));

    m1 = (p21 - p11) * cf;
    if m1 > 0
        e1 = p11 - m1;
        xir = ir;
    else
        e1 = p21 + m1;
        xir = 1 - ir;
    end

    m2 = (p12 - p11) * cf;
    if m2 > 0
        e2 = p11 - m2;
        yir = ir;
    else
        e2 = p12 + m2;
        yir = 1 - ir;
    end
    
    e = e1 - p11 + e2;
    
    %plot3(xi(x) + 0.5 * (xi(x+1) - xi(x)), yi(y), e1, '.g', 'MarkerSize', 20);
    %plot3(xi(x), yi(y) + 0.5 * (yi(y+1) - yi(y)), e2, '.g', 'MarkerSize', 20);
    color = "r";
    if e <= minz
        color = "g";
        xn = floor(xi(x) + xir * (xi(x+1) - xi(x)));
        yn = floor(yi(y) + yir * (yi(y+1) - yi(y)));

        %cs = [xi(x), yn; xi(x+1), yn; xn, yi(y); xn, yi(y+1); xn, yn];
        xs = [xi(x), xn, xi(x+1)];
        ys = [yi(y), yn, yi(y+1)];
        [X, Y] = meshgrid(xs, ys);
        surf(X', Y', d(xs, ys), 'EdgeColor', 'g', 'LineWidth', 2);
        alpha 0
        
%         a = zeros(1, 5);
%         for i = 1:5
%             a(i) = d(cs(i, 1), cs(i, 2));
%         end
%         
%         points = cs;
%         plot3(cs(:, 1), cs(:, 2), a, '.m', 'MarkerSize', 20);
    end
   
    %plot3(xi(x) + xir * (xi(x+1) - xi(x)), yi(y) + yir * (yi(y+1) - yi(y)), e, '.', 'Color', color, 'MarkerSize', 20);
end

