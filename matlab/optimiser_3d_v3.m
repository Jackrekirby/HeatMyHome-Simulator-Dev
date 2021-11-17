clear all;
clc;
for w = 1  
    w
    d = readmatrix(sprintf("c_surfaces/%i.csv", w));

    % ALGORITHM
    global b;

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
    aa = d(xi, yi);
    dx = diff(d(xi, yi), 1, 1);
    dy = diff(d(xi, yi), 1, 2);
    
    mdx = 0;
    mdxc = [0, 0];
    for j = 1:size(dx, 2)
        for i = 1:size(dx, 1)
            if mdx < abs(dx(i, j))
                mdx = abs(dx(i, j));
                mdxc = [i, j];
            end
        end
    end
    
    mdy = 0;
    mdyc = [0, 0];
    for j = 1:size(dy, 2)
        for i = 1:size(dy, 1)
            if mdy < abs(dy(i, j))
                mdy = abs(dy(i, j));
                mdyc = [i, j];
            end
        end
    end
    
    hx = mdx / (xi(mdxc(1) + 1) - xi(mdxc(1)))
    hy = mdy / (yi(mdyc(1) + 1) - yi(mdyc(1)))
    
    mdxcm = floor(interp(xi(mdxc(1)), xi(mdxc(1) + 1)));
    mdycm = floor(interp(yi(mdyc(2)), yi(mdyc(2) + 1)));
    plot3(mdxcm, yi(mdxc(2)), d(mdxcm, yi(mdxc(2))), 'oc', 'LineWidth', 3);
    plot3(xi(mdyc(1)), mdycm, d(xi(mdyc(1)), mdycm), 'om', 'LineWidth', 3);
    
    xlabel('XXX');
    ylabel('YYY');
    zlabel('ZZZ');
    
    min_val = min(d(xi, yi), [], 'all');
%     break;
%     [min_val, min_ind] = min(d(xi, yi), [], 'all', 'linear');
%     [max_val, max_ind] = max(d(xi, yi), [], 'all', 'linear');
%     
%     max_xi = xi(mod(max_ind - 1, length(xi)) + 1);
%     min_xi = xi(mod(min_ind - 1, length(xi)) + 1);
%     max_yi = yi(floor((max_ind - 1)/ length(yi)) + 1);
%     min_yi = yi(floor((min_ind - 1)/ length(yi)) + 1);
%     
%     hx = (max_val - min_val) / abs(max_xi - min_xi);
%     hy = (max_val - min_val) / abs(max_yi - min_yi);
%     plot3(max_xi, max_yi, max_val, 'oc', 'LineWidth', 3);
%     plot3(min_xi, min_yi, min_val, 'ok', 'LineWidth', 3);

    Xs = []; Ys = [];
    for j = 1:length(yi)-1
        for i = 1:length(xi)-1
            [xs, ys] = plane(d, cf, xi, yi, i, j, min_val, hx, hy);
            Xs = [Xs; xs];
            Ys = [Ys; ys];
        end
    end
    view([135 30]);
    hold off;
    %break;
    pause;

    while ~isempty(Xs)
%         [min_val, min_ind] = min(d(xi, yi), [], 'all', 'linear');
%         [max_val, max_ind] = max(d(xi, yi), [], 'all', 'linear');
% 
%         max_xi = xi(mod(max_ind - 1, len(xi)) + 1);
%         min_xi = xi(mod(min_ind - 1, len(xi)) + 1);
%         max_yi = yi(floor((max_ind - 1)/ len(yi)) + 1);
%         min_yi = yi(floor((min_ind - 1)/ len(yi)) + 1);
% 
%         hx = (max_val - min_val) / abs(max_xi - min_xi);
%         hy = (max_val - min_val) / abs(max_yi - min_yi);
        
        Xs2 = []; Ys2 = [];
        surf(X', Y', d, 'FaceAlpha', 0.2);
        hold on;
        for k = 1:size(Xs, 1)
            for j = 1:2
                for i = 1:2
                    [xs, ys] = plane(d, cf, Xs(k, :), Ys(k, :), i, j, min_val, hx, hy);
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

    [~, minima_i] = min(d, [], 'all', 'linear');
    its = nnz(b)
    efficiency = its / (s(1) * s(2)) * 100
    found = b(minima_i) == 1
    pause;
end

function c = interp(a, b)
    c = a + 0.5 * (b - a);
end

function [xs, ys] = plane(d, cf, xi, yi, x, y, min_val, hx, hy)
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
        p22 = d(xi(x+1), yi(y+1));
        
        low_val = min([p11, p21, p12, p22]);

        dx = xi(x+1) - xi(x);
        dy = yi(y+1) - yi(y); 
        e = low_val - ((hx * dx + hy * dy) * cf);
        
        
                
%         m1 = (p21 - p11) * cf;
%         if m1 > 0
%             e1 = p11 - m1;
%             xir = ir;
%         else
%             e1 = p21 + m1;
%             xir = 1 - ir;
%         end
% 
%         m2 = (p12 - p11) * cf;
%         if m2 > 0
%             e2 = p11 - m2;
%             yir = ir;
%         else
%             e2 = p12 + m2;
%             yir = 1 - ir;
%         end

        %e = e1 - p11 + e2;

        if e <= min_val
            color = 'g';
            xn = floor(xi(x) + 0.5 * (xi(x+1) - xi(x)));
            yn = floor(yi(y) + 0.5 * (yi(y+1) - yi(y)));

            xs = [xi(x), xn, xi(x+1)];
            ys = [yi(y), yn, yi(y+1)];
            [X, Y] = meshgrid(xs, ys);
            surf(X', Y', d(xs, ys), 'EdgeColor', color, 'LineWidth', 2, 'FaceAlpha', 0);
        else
            color = 'r';
            xn = floor(xi(x) + 0.5 * (xi(x+1) - xi(x)));
            yn = floor(yi(y) + 0.5 * (yi(y+1) - yi(y)));

            xs2 = [xi(x), xn, xi(x+1)];
            ys2 = [yi(y), yn, yi(y+1)];
            [X, Y] = meshgrid(xs2, ys2);
            surf(X', Y', d(xs2, ys2), 'EdgeColor', color, 'LineWidth', 2, 'FaceAlpha', 0);
        end
    
    end
    %alpha 0
end

