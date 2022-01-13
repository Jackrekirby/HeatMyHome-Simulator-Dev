%% 3D OPTIMISER EFFICIENCY - SUCCESS RATE, WORKING
clear all;
clc;
global b;
t_found = 0;
ran = 0:2120;
t_efficiency = 0;
cf = 0.2;
tot_its = 0;

% CF	% Reduction	% Found
% 1	27.4	100
% 0.5	18.5	100
% 0.2	12.1	100
% 0.1	9.5	99.94
% 0	6.7	97.47


for w = ran
    d = readmatrix(sprintf("c_surfaces/%i.csv", w));
    s = size(d);
    if s(1) == 1
        continue;
    end
    
    tot_its = tot_its + 1;
    b = zeros(s(1), s(2));

%     [X,Y] = meshgrid(1:s(1), 1:s(2));
%     surf(X', Y', d, 'FaceAlpha', 0.2);
%     hold on;

    xi = floor(linspace(1, s(1), 5));
    yi = floor(linspace(1, s(2), 5));
%     [x,y] = meshgrid(xi, yi);
%     plot3(x', y', d(xi, yi), '.b', 'MarkerSize', 20);
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
    
    hx = mdx / (xi(mdxc(1) + 1) - xi(mdxc(1)));
    hy = mdy / (yi(mdyc(2) + 1) - yi(mdyc(2)));
    
    mdxcm = floor(interp(xi(mdxc(1)), xi(mdxc(1) + 1)));
    mdycm = floor(interp(yi(mdyc(2)), yi(mdyc(2) + 1)));
%     plot3(mdxcm, yi(mdxc(2)), d(mdxcm, yi(mdxc(2))), 'oc', 'LineWidth', 3);
%     plot3(xi(mdyc(1)), mdycm, d(xi(mdyc(1)), mdycm), 'om', 'LineWidth', 3);
%     
%     xlabel('XXX');
%     ylabel('YYY');
%     zlabel('ZZZ');
    
    [min_val, min_ind] = min(d, [], 'all', 'linear');

    cur_min_val = min(d(xi, yi), [], 'all');
    Xs = []; Ys = [];
    for j = 1:length(yi)-1
        for i = 1:length(xi)-1
            [xs, ys] = plane(d, cf, xi, yi, i, j, cur_min_val, hx, hy);
            Xs = [Xs; xs];
            Ys = [Ys; ys];
        end
    end
%     view([135 30]);
%     hold off;
    %break;
    %pause;

    while ~isempty(Xs)
        Xs2 = []; Ys2 = [];
        cur_min_val = min(d(Xs, Ys), [], 'all');
%         surf(X', Y', d, 'FaceAlpha', 0.2);
%         hold on;
        for k = 1:size(Xs, 1)
            for j = 1:2
                for i = 1:2
                    [xs, ys] = plane(d, cf, Xs(k, :), Ys(k, :), i, j, cur_min_val, hx, hy);
                    Xs2 = [Xs2; xs];
                    Ys2 = [Ys2; ys];
                end
            end
        end
        Xs = Xs2;
        Ys = Ys2;
%         view([135 30]);
%         hold off;
%         pause;
    end

    its = nnz(b);
    efficiency = its / (s(1) * s(2)) * 100;
    found = b(min_ind) == 1;
    fprintf('N=%i, \t REDUCTION_PERC=%.1f, \t FOUND=%i\n', w, efficiency, found);
    t_found = t_found + found;
    t_efficiency = t_efficiency + efficiency;
%     pause;
end

fprintf('NUM_SURFACES=%i, CF=%.2f, REDUCTION_PERC=%.1f, FOUND=%i, PERC_FOUND=%.2f\n', tot_its, cf, t_efficiency / tot_its, t_found, (t_found / tot_its) * 100);



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

        if e <= min_val
            color = 'g';
            xn = floor(xi(x) + 0.5 * (xi(x+1) - xi(x)));
            yn = floor(yi(y) + 0.5 * (yi(y+1) - yi(y)));

            xs = [xi(x), xn, xi(x+1)];
            ys = [yi(y), yn, yi(y+1)];
%             [X, Y] = meshgrid(xs, ys);
%             surf(X', Y', d(xs, ys), 'EdgeColor', color, 'LineWidth', 2, 'FaceAlpha', 0);
%             plot3(xn, yn, e, 'go'); 
        else
%             color = 'r';
%             xn = floor(xi(x) + 0.5 * (xi(x+1) - xi(x)));
%             yn = floor(yi(y) + 0.5 * (yi(y+1) - yi(y)));
% 
%             xs2 = [xi(x), xn, xi(x+1)];
%             ys2 = [yi(y), yn, yi(y+1)];
%             [X, Y] = meshgrid(xs2, ys2);
%             surf(X', Y', d(xs2, ys2), 'EdgeColor', color, 'LineWidth', 2, 'FaceAlpha', 0);
%             plot3(xn, yn, e, 'ro'); 
        end
    
    end
    %alpha 0
end

