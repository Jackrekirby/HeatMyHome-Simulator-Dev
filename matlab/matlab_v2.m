%% COLLECTION OF NON FUNCTIONAL CODE
clear all
clc

data = readtable('output.txt');
data = data{:,:};
disp('imported')

%% HEAT, SOLAR, SOLARSIZE, TES, TARIFF, NPC
clc;
no_tariffs = 5;
no_points = length(data);
no_tes_volumes = 30;
npc_per_tariff = zeros(no_points/no_tariffs, no_tariffs);

npc_index = 6;
for tariff = 1:no_tariffs
    npc_per_tariff(:, tariff) = data(tariff:no_tariffs:no_points, npc_index);
end

min_tariff_npc_vec = min(npc_per_tariff')';
min_tariff_npc = reshape(min_tariff_npc_vec, no_tes_volumes, []);

solar_size_index = 3;
solar_size_mat = reshape(data(tariff:no_tariffs:no_points, solar_size_index), no_tes_volumes, []);
solar_size = solar_size_mat(1, :); % solar_size for each tes is the same


surfaces = {};
j = 0;
start_index = 1;
for i = 1:length(solar_size)
    %fprintf("i %i, %i\n", i, solar_size(i))
    if i == length(solar_size) || solar_size(i+1) == 0
        j = j + 1;
        %fprintf("   j %i\n", j)
        surfaces{j} = min_tariff_npc(:, start_index:i);
        start_index = i + 1;
    end
end

j = 0;
k = 0;
for i = 1:length(surfaces)
    if size(surfaces{i}, 2) == 1
        j = j + 1;
        writematrix(surfaces{i},sprintf('lines/%i.csv', j));
    else
        k = k + 1;
        writematrix(surfaces{i},sprintf('surfaces/%i.csv', k));
    end
end

%%
%tiledlayout(7,3);
figure('units','normalized','outerposition',[0 0 1 1]);
colormap hsv
for i = 1:length(surfaces)
    %nexttile
    if size(surfaces{i}, 2) == 1
        plot(0:29, surfaces{i});
        xlabel('TES Volume');
        ylabel('NPC');
        xlim([0, 29]);
    else
        [x,y] = meshgrid(0:29, 0:size(surfaces{i}, 2) -1);
        surf(x', y', surfaces{i});
        
        ylabel('Solar Size');
        xlabel('TES Volume');
        zlabel('NPC');
        
        xlim([0, 29]);
        ylim([0, (size(surfaces{i}, 2) -1)]);
    end
    
    
            
    pause(0.2)
    
    saveas(gcf,sprintf('images/%i.png', i))
end


disp('done')