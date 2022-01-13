%% 3D OPTIMISER VISUALISER, WORKING BUT POSSIBLY BUGGY
clear all;
clc;

for w = 1
    data = readmatrix(sprintf("c_surfaces/%i.csv", w));
    %disp('imported');

    % ALGORITHM
    global b;

    d = data;
    s = size(d);

    b = zeros(s(1), s(2));

    [X,Y] = meshgrid(1:s(1), 1:s(2));
    surf(X', Y', d, 'FaceAlpha', 0.2);
    v = view;
    hold on;

    xi = floor(linspace(1, s(1), 5));
    yi = floor(linspace(1, s(2), 5));
    [x,y] = meshgrid(xi, yi);
    plot3(x', y', d(xi, yi), '.b', 'MarkerSize', 20);

    cf = 1;
    ir = 0.5;

    [minz, minima_i] = min(d, [], 'all', 'linear');
    Xs = []; Ys = [];
    for j = 1:length(yi)-1
        for i = 1:length(xi)-1
            [xs, ys] = plane(d, cf, ir, xi, yi, i, j, minz);
            Xs = [Xs; xs];
            Ys = [Ys; ys];
        end
    end
    view([135 30]);
    plot3(mod(minima_i - 1, s(1)) + 1, floor((minima_i-1) / s(1)) + 1, minz, 'co', 'LineWidth', 3);
    hold off;

    pause;

    while ~isempty(Xs)
        Xs2 = []; Ys2 = [];
        surf(X', Y', d, 'FaceAlpha', 0.2);
        hold on;
        for k = 1:size(Xs, 1)
            for j = 1:2
                for i = 1:2
                    [xs, ys] = plane(d, cf, ir, Xs(k, :), Ys(k, :), i, j, minz);
                    Xs2 = [Xs2; xs];
                    Ys2 = [Ys2; ys];
                end
            end
        end
        Xs = Xs2;
        Ys = Ys2;
        view([135 30]);
        plot3(mod(minima_i - 1, s(1)) + 1, floor((minima_i-1) / s(1)) + 1, minz, 'co', 'LineWidth', 3);
        hold off;
        pause;
    end

    its = nnz(b);
    efficiency = its / (s(1) * s(2)) * 100;
    found = b(minima_i) == 1;
    fprintf('n=%i, eff=%.1f, found=%i\n', w, efficiency, found);
    pause;
end

function [xs, ys] = plane(d, cf, ir, xi, yi, x, y, minz)
    global b;
    xs = [];
    ys = [];
    b(xi(x), yi(y)) = 1;
    b(xi(x+1), yi(y)) = 1;
    b(xi(x), yi(y+1)) = 1;
    b(xi(x+1), yi(y+1)) = 1;
    if xi(x+1) - xi(x) > 1 || yi(y+1) - yi(y) > 1
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

        if e <= minz
            color = 'g';
            xn = floor(xi(x) + xir * (xi(x+1) - xi(x)));
            yn = floor(yi(y) + yir * (yi(y+1) - yi(y)));

            xs = [xi(x), xn, xi(x+1)];
            ys = [yi(y), yn, yi(y+1)];
            [X, Y] = meshgrid(xs, ys);
            surf(X', Y', d(xs, ys), 'EdgeColor', color, 'LineWidth', 2, 'FaceAlpha', 0);
        else
            color = 'r';
            xn = floor(xi(x) + xir * (xi(x+1) - xi(x)));
            yn = floor(yi(y) + yir * (yi(y+1) - yi(y)));

            xs2 = [xi(x), xn, xi(x+1)];
            ys2 = [yi(y), yn, yi(y+1)];
            [X, Y] = meshgrid(xs2, ys2);
            surf(X', Y', d(xs2, ys2), 'EdgeColor', color, 'LineWidth', 2, 'FaceAlpha', 0);
        end
    
    end
    %alpha 0
end

