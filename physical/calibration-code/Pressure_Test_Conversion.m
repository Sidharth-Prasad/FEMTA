C = 6.114/2^15*2.2;

% Rig Pressure Sensor
S1 = [-8.05518095144006e-06 0.000179617025688186 -0.00149263466053365 0.0137083951784765 3.84928878749102 -0.389869854380195];
press_1 = importdata('ad15-4a.log');
Press_data = press_1(:,1);
Tank_press = S1(1).*(Press_data.*C).^5+S1(2).*(Press_data.*C).^4+S1(3).*(Press_data.*C).^3+S1(4).*(Press_data.*C).^2+S1(5).*(Press_data.*C)+S1(6);

%Ambient Pressure Sensor
S3 = [-7.76498003060808e-06 0.000162676991666468 -0.00125702109568652 0.0131681178015827 3.84252993123933 -0.402960357438964];
press_3 = importdata('ad15-49.log');
Press_3 = press_3(:,2);
Ambient_press = S3(1).*(Press_3.*C).^5+S3(2).*(Press_3.*C).^4+S3(3).*(Press_3.*C).^3+S3(4).*(Press_3.*C).^2+S3(5).*(Press_3.*C)+S3(6);

% Temperature Data and Calibration
temp = importdata('ad15-48.log');
A = 0.0007898768;
B = 0.0002715943;
D = 1.1181236E-07;
R_1 = (temp(:,1) .* 6.114 / 2^15) ./ 0.0000567;
%R_2 = (temp(:,2) .* 6.114 / 2^15) ./ 0.0000567;
temp_1 = 1 ./ (A + B .* log(R_1) + D .* (log(R_1)) .^3);
%temp_2 = 1 ./ (A + B .* log(R_2) + D .* (log(R_2)) .^3);

% Plotting
grid on
figure(1)
hold on

yyaxis left
ylabel('Pressure (kPa)')
plot(Tank_press, 'r')
plot(Ambient_press)

yyaxis right
ylabel('Temperature (K)')
plot(temp_1, 'g')
%plot(temp_2, 'k')

hold off
legend('Chamber Pressure', 'Ambient Pressure','Liquid Temp','location','best')
xlabel('Time Elapsed')
title('Pressure Test')