clear all;
for w = 1:18
    clc;
    data = readtable(sprintf("surfaces_xl/%i.csv", w));
    data = data{:,:};
    %disp('imported');

    % ALGORITHM
    clc;
    global b;

    d = data;
    s = size(d);

    b = zeros(s(1), s(2));

    [X,Y] = meshgrid(1:s(1), 1:s(2));
    surf(X', Y', d, 'FaceAlpha', 0.2);
    hold on;

    xi = floor(linspace(1, s(1), 5));
    yi = floor(linspace(1, s(2), 5));
    [x,y] = meshgrid(xi, yi);
    plot3(x', y', d(xi, yi), '.b', 'MarkerSize', 20);

    cf = 0.01;
    ir = 0.5;

    minz = min(d(xi, yi));
    Xs = []; Ys = [];
    for j = 1:length(yi)-1
        for i = 1:length(xi)-1
            [xs, ys] = plane(d, cf, ir, xi, yi, i, j, minz);
            Xs = [Xs; xs];
            Ys = [Ys; ys];
        end
    end
    view([135 30]);
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
        hold off;
        pause;
    end

    its = nnz(b)
    efficiency = its / (s(1) * s(2)) * 100
    pause;
end

function [xs, ys] = plane(d, cf, ir, xi, yi, x, y, minz)
    global b;
    xs = [];
    ys = [];
    if xi(x+1) - xi(x) > 1 || yi(y+1) - yi(y) > 1
        p11 = d(xi(x), yi(y));
        p21 = d(xi(x+1), yi(y));
        p12 = d(xi(x), yi(y+1));
        b(xi(x), yi(y)) = 1;
        b(xi(x+1), yi(y)) = 1;
        b(xi(x), yi(y+1)) = 1;

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

