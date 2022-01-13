%% CONCEPT TEST - INTERSECTION OF SEGMENTS - INCOMPLETE BUT WORKING
clear all;
clc;

x = 0:10;
y = x.^3 - 9*x.^2 - 30*x + 200;

plot(x, y, '.-b');
hold on;


dy = y(2) - y(1)
dy2 = y(end) - y(end-1)

p = length(x);

plot(x, y(1) + dy * x);
c = y(p) + dy2 * x(p)
plot(x, y(p) + dy2 * x - c);
% y1 + m1*x = y2 - m2*x
% (m1 + m2)x = (y2 - y1)
% (y2 - y1)/(m1 + m2) = x
x2 = (y(end) - y(1)) / (dy2 + dy)




% for i = 1:p/2
%     q = floor(linspace(1, p, i));
%     plot(x(q), y(q), '.-');
% end
%colororder(hsv(p));
hold off;