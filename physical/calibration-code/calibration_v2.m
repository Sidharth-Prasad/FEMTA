close all;

%import raw sensor data from .csv file
rawData = importdata('rawPressureData.csv');
data_01 = rawData.data(:,1);
data_02 = rawData.data(:,2);
data_03 = rawData.data(:,3);
dataSize = size(rawData.data,1);

%import recorded pressure data from .xlsx file (excel)
rawData = importdata('stepPressure.xlsx');
manoData = rawData.data;
%following 2 lines aligns 3 columns of data into one column with no gaps in between
manoData = [manoData(:,1);manoData(:,2);manoData(:,3)];
manoData(~any(~isnan(manoData), 2),:)=[];

%performing analysis on all 3 raw data sets
dataAnalysis(manoData,data_01,dataSize,1000,200,200,1,2,3,1);
dataAnalysis(manoData,data_02,dataSize,2000,125,100,1,2,3,2);
dataAnalysis(manoData,data_03,dataSize,1000,200,200,1,2,3,3);

%dataAnalysis function:
%   manoData: actual pressure data in kPa
%   data: raw sensor data (arbitrary units)
%   dataSize: size of 'data'
%   slopeMultiplier: because of the scale of the y-axis, slope must be scaled to allow for more precise 'valid data' detection
%   slopeTrigger: when slope <= this point, data is 'valid'
%   valueTrigger: serves as an exception to the 'slope rule'. If actual data value difference between points is <= this value, retains data as 'valid'
%   regDegree: degree of regression fit to generate (broken atm, use the 'Basic Fitting' function in plot tools when graph generates)
%   subplotM: subplot row dimension
%   subplotN: subplot column dimension
%   subplotP: this graph's subplot index number
%
function dataAnalysis(manoData,data,dataSize,slopeMultiplier,slopeTrigger,valueTrigger,regDegree,subplotM,subplotN,subplotP)
    %basic moving-average noise reduction
    data = smooth(data,1000);
    %generating slope data (with multiplier)
    for i=2:dataSize
        slope(i) = slopeMultiplier*(data(i)-data(i-1));
    end
    slope = smooth(slope,10);
    
    %finding 'valid data' ranges -> the term 'valid' here describes 'useful' since we want to match the plateaus on the sensor data graph with our recorded pressure data
    last(1) = 1;
    last(2) = data(1);
    for i=1:dataSize
        if slope(i) <= slopeTrigger && slope(i) > 0
            if abs(data(i) - last(2)) < valueTrigger %even if data does a little 'dip' along a plateau (resulting in a negative slope), if the data isn't far off the last 'valid' data point, we retain all data in between as 'valid'
                for j=last(1):i
                    binData(j) = data(j);
                end
            end
            binData(i) = data(i);
            last(1) = i;
            last(2) = data(i);
        else
            binData(i) = 0;
        end
    end
    
    %copying over to another matrix to perform final analysis (not really sure the reason for the fancy floop and if-statement)
    for i=1:dataSize
        if(binData(i)==0)
            newData(i) = 0;
        else
            newData(i) = data(i);
        end
    end
    
    %performing smoothing of 'plateau' regions of the 'valid' data
    mode = 1; %1 -> finding valid data start, 0 -> finding valid data end
    leadInIDX = 1;
    for i=1:dataSize
        if mode == 1
            if newData(i) ~= 0
                mode = 0;
                leadInIDX = i;
            end
        elseif mode == 0
            if newData(i) == 0
                mode = 1;
                %take weighted average of all data points in range (latter points weighted more)
                avg = sum(newData(leadInIDX:i-1).*(1:i-leadInIDX))/((i-leadInIDX)+sum(1:i-leadInIDX));
                newData(leadInIDX:i-1) = avg;
            end
        end
    end
    
    %generates a new matrix with only data of the 'valid' data plateau regions. The length of this should closely match the length of 'manoData'
    last = -1;
    cntr = 1;
    snsrData = zeros; %zero vector out if variable in workspace
    for i=1:dataSize
        if(last ~= newData(i) && newData(i) ~= 0)
            snsrData(cntr,1) = newData(i);
            cntr = cntr + 1;
            last = newData(i);
        end
    end
    
    %finally matching recorded pressure data with the raw sensor data
    if size(manoData,1) <= size(snsrData,1)
        conversionData(:,1) = snsrData(1:size(manoData,1));
        conversionData(:,2) = manoData(:);
    else
        conversionData = [0,0];
        fprintf('No conversion data. Sensor data matrix too small.\n');
        fprintf('snsrData: %i, manoData: %i\n',size(snsrData,1),size(manoData,1));
    end
    
    %finding line of best fit with desired degree
    conversionFuncCoefs = polyfit(conversionData(:,1),conversionData(:,2),regDegree);
    conversionFunc = polyval(conversionFuncCoefs,conversionData(:,1));
    fprintf('Linear Regression Equation: %0.4fx + %0.4f\n',conversionFuncCoefs(1),conversionFuncCoefs(2));
    
    %plotting shenanigans
    subplot(subplotM,subplotN,subplotP);
    hold on;
    plot(data);
    axis manual;
    plot(slope);
    plot(binData);
    plot(newData);
    title('Sensor Data Analysis');
    xlabel('Time Index');
    ylabel('Raw Sensor Data');
    legend({'data','slope','binData','newData'});
    hold off;
    subplot(subplotM,subplotN,subplotP+subplotN);
    hold on;
    axis auto;
    scatter(conversionData(:,1),conversionData(:,2),'.');
    plot(conversionData(:,1),conversionFunc);
    legend({'conversionData','linear reg.'});
    title('Conversion Plot');
    xlabel('Raw Sensor Data');
    ylabel('kPa');
    hold off;
end
