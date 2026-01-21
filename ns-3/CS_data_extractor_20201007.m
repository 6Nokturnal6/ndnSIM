%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% This script is used to extract data from ndnSIM L3Tracer%
% Wrote: Elidio Tomas da Silva                            %
% 2019-12-07                                              %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Specify the folder where the files live.
baseDataFolder = '/home/dasilva/PDEEC2020/Cache_20201027/scenario_5/cs-pdeec/';
% Get a list of all folder with the desired file name pattern.
dataDirectories=dir(fullfile(baseDataFolder, 'cs-*'));
%dataDirectories=dir(fullfile(baseDataFolder, 'new*'));


for j = 1 : length(dataDirectories)
% Check to make sure that folder actually exists.  Warn user if it doesn't.
if ~isfolder(dataDirectories(j).name)
  errorMessage = sprintf('Error: The following folder does not exist:\n%s', dataDirectories(j).name);
  uiwait(warndlg(errorMessage));
  return;
end

fprintf(1, '\n\n>>>>>> %s\n\n', fullfile(baseDataFolder,dataDirectories(j).name));
cd(dataDirectories(j).name)

%Remove un-needed lines (The tittle line and the '...-1 all' line)
% % the script: bash
% % for i in $( ls rate-pdeec-*_20191201*.txt ); do
% %     sed -i '/FaceDescr\|all/d' $i; 
% % done
pathToScript = fullfile(pwd,'remove_first.sh'); %here
%some_arg = 'arg1';
%another_arg = 'arg2';
cmdStr = pathToScript; %[pathToScript ' ' some_arg ' ' another_arg];
system(cmdStr);

% Get a list of all files in the folder with the desired file name pattern.
filePattern = fullfile(baseDataFolder,dataDirectories(j).name, 'cs-*.txt'); % Change to whatever pattern you need.
theFiles = dir(filePattern);
 
fprintf(1, '\nEntering the folder->> %s\n\n', dataDirectories(j).name);
t = 0;
for k = 1 : length(theFiles)
  baseFileName = theFiles(k).name;
  fullFileName = fullfile(dataDirectories(j).name, baseFileName);
  fprintf(1, 'Reading ->> %s\n', baseFileName);
  [fid, message1] = fopen(baseFileName,'r');

  
% read the data from ndnSIM produced file
  %datavector = textread(baseFileName, '%*u%*s%*u%*s%*s%*f%*f%u%*f', 'endofline','\n');%packet Raw
  data = textread(baseFileName, '%*u%*s%*s%u', 'endofline','\n');
  %frewind(fid);
  fid=fclose(fid);
  %dados = data;
  hits = data(1:2:end);% extract the hit
  miss = data(2:2:end);% extract the miss

  
  % Calculate Cache Hit Ratio per simulation
  
  
  CacheHitRatio = sum(hits)/(sum(hits)+sum(miss))*100;
  [fidParcial, message3] = fopen('aggregatedRatio.txt','a+');
  if (t == 0)
      fprintf(fidParcial, '%s\n', dataDirectories(j).name);
  end
  fprintf(fidParcial, '%2.4f\n', CacheHitRatio); 
  fclose(fidParcial);
  t = 1;
  
  % Aggregate data, in two columns
  [fidin, message2] = fopen('aggregatedData.txt','a+');
  fprintf(fidin, '%3u %3u\n', [hits miss]'); 
  fclose(fidin);
end
movefile 'aggregatedRatio.txt' 'aggregatedRatio.txt.backup'
% Import aggregated data to workspace (InInterest and OutData)
Data = importdata('aggregatedData.txt'); 
%Data = data;
%  hits = Data(1:length(Data),1); % the first column
%  miss = Data(1:length(Data),2); % the second column

% fprintf(1, 'write data to * %s * file\n', 'Random_L3_100.xls');
% save Random_L3_100libre.xls data -ascii;

fprintf(1, 'write data to * %s * file\n', 'OutputToSPSS.{csv,xls}');
xlswrite('OutputToSPSS.xls', Data); % Import all to *.{xls,csv}

%backup this *.txt so it will not be accidentally processed with data next time
movefile 'aggregatedData.txt' 'aggregatedData.txt.backup'

% Calculate Cache Hit Ratio
CacheHitRatio = sum(hits)/(sum(hits)+sum(miss))*100

fprintf(1, '\n\n<<<<<< %s\n\n', fullfile(baseDataFolder,dataDirectories(j).name));
cd(baseDataFolder); % cd to baseDir

end
% better clear the environment!...
clear;
