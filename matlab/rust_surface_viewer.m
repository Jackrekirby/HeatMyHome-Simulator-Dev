% SURFACE VIEWER AND SAVER - FULLY WORKING
clear all; clc;
cd
dir = "../rust_simulator/tests/surfaces";

i = 0;
figure;
for w = 463
    disp(w);
    filename = sprintf("%s/%i.csv", dir, w);
    if isfile(filename)
        d = readmatrix(filename);
        d = d / 1000;
        s = size(d);
        if min(s) > 1
            filename2 = sprintf("%s/o%i.csv", dir, w);
            if isfile(filename2)
                f = readmatrix(filename2);
                f = f / 1000;
                [X,Y] = meshgrid(1:s(1), 1:s(2));
                X = X * 2;
                Y = Y * 0.1;
                X = X';
                Y = Y';
                rr = 0.3;
                surf(X, Y, d, 'EdgeColor', [rr, rr, rr]);
                alpha 0.5
                [val, ind] = min(d(:))
                hold on;
                xlim([0, 40]);
                ylim([0, 3.0]);
                X2 = X;
                X2(ind) = NaN;
                Y2 = Y;
                Y2(ind) = NaN;
                pp1 = plot3(X2, Y2, f, 'r.', 'MarkerSize', 15, 'LineWidth', 3);
                pp2 = plot3(X(ind), Y(ind), val, 'g.', 'MarkerSize', 25, 'LineWidth', 3);
                zticks([13:16]);
                hold off;
                pp = [pp1(1); pp2];
                legend(pp, ["Searched Nodes", "Global Minimum"], "location", "northeast");
                %title(sprintf('Surface: %i', w));
                set(gca, 'FontName', 'FixedWidth', 'FontWeight', 'Bold')
                ylabel("TES Size (m^3)");
                xlabel("Solar Size (m^2)");
                zlabel("Lifetime Cost (£1000's)");
                view([45, 30]);
                set(get(gca,'xlabel'),'rotation',-20); %where angle is in degrees
                set(get(gca,'ylabel'),'rotation',20); %where angle is in degrees
                i = i + 1;
                %saveas(gcf,sprintf("rust_images/%i.png", i));
                if length(w) > 1
                    pause;
                end
                %pause
            end
        end
    end
end