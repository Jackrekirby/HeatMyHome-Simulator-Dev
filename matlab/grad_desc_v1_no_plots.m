%% GRADIENT DESCENT ALGORITHM, SUCCESS-EFFICIENCY TEST, WORKING
clear all;
clc;
global b
global c
% figure;


ran = 0:2120;
t_found = 0;
t_reduction = 0;
tot_its = 0;
gs = 100;

for di = ran
    d = readmatrix(sprintf("c_surfaces/%i.csv", di));
    sd = size(d);
    if sd(1) == 1
        continue;
    end
    tot_its = tot_its + 1;
%     [sx,sy] = meshgrid(1:sd(1), 1:sd(2));

    b = zeros(sd(1), sd(2));
    c = zeros(sd(1), sd(2));

    xis = 1:gs:sd(1);
    yis = 1:gs:sd(2);
    step = 1;
    for yj = 1:length(yis)
        for xj = 1:length(xis)
            xi = xis(xj);
            yi = yis(yj);
            for i = 1:100
%                 surf(sx', sy', d, 'FaceAlpha', 0.2);
%                 hold on;
                [xi, yi, stop] = get_dir(d, xi, yi, step);
                if stop
                    break;
                end
%                 hold off;
%                 xlabel('XXX');
%                 ylabel('YYY');
%                 zlabel('ZZZ');
%                 pause(0.005);
            end
        end
    end

    points_searched = nnz(c);
    reduction = points_searched / (sd(1) * sd(2)) * 100;
    [min_val, min_ind] = min(d, [], 'all', 'linear');
    found = c(min_ind) == 1;

    t_found = t_found + found;
    t_reduction = t_reduction + reduction;
    fprintf('I=%i, N=%i, \t REDUCTION_PERC=%.1f, \t FOUND=%i\n', di, points_searched, reduction, found);
end
fprintf('NUM_SURFACES=%i, GRID_STEP=%i, REDUCTION_PERC=%.1f, FOUND=%i, PERC_FOUND=%.2f\n', tot_its, gs, t_reduction / tot_its, t_found, (t_found / tot_its) * 100);



function [xi2, yi2, stop] = get_dir(d, xi, yi, step)
    global b
    global c
    if b(xi, yi) == 1
        xi2 = 0;
        yi2 = 0;
        stop = true;
        return;
    else
        b(xi, yi) = 1;
        c(xi, yi) = 1;
        stop = false;
    end
    
    p11 = d(xi, yi);
    sd = size(d);
    
    xstep = step;
    if xi == sd(1)
        xin = xi - xstep;
    else
        xin = xi + xstep;
    end
    
%     while xstep > 1 && c(xin, yi) == 1
%         xstep = xstep - 1;
%         if xi == sd(1)
%             xin = xi - xstep;
%         else
%             xin = xi + xstep;
%         end
%     end
%     
    ystep = step;
    if yi == sd(2)
        yin = yi - ystep;
    else
        yin = yi + ystep;
    end
    
%     while ystep > 1 && c(xi, yin) == 1
%         ystep = ystep - 1;
%         if yi == sd(1)
%             yin = yi - ystep;
%         else
%             yin = yi + ystep;
%         end
%     end
    
    p21 = d(xin, yi);
    p12 = d(xi, yin);
    c(xin, yi) = 1;
    c(xi, yin) = 1;

%     lw = 2;
%     plot3(xi, yi, p11, 'go', 'LineWidth', lw);
%     plot3(xin, yi, p21, 'bo', 'LineWidth', lw);
%     plot3(xi, yin, p12, 'ro', 'LineWidth', lw);
%     
    dx = (p21 - p11) ;
    dx = dx / (xi - xin);
    dy = (p12 - p11);
    dy = dy / (yi - yin);
    
    sdx = sign(dx);
    sdy = sign(dy);
        
    xi2 = xi + sdx * step;
    yi2 = yi + sdy * step;
    
    if xi2 >= sd(1)
        xi2 = sd(1);
    elseif xi2 < 1
        xi2 = 1;
    end
    
    if yi2 >= sd(2)
        yi2 = sd(2);
    elseif yi2 < 1
        yi2 = 1;
    end
    
%     plot3(xi2, yi2, d(xi2, yi2), 'mo', 'LineWidth', lw);
    
end