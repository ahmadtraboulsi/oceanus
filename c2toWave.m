function []=c2toWave(iFile,oFile,f,toffset,oSps,BitsPerSample)
    % Convert a baseband signal (".c2" file format, complex envelope) to
    % broadband signal (".wav" file format).
    %
    % Author: Michel Barbeau, Carleton University
    % Version: July 8, 2019
    % iFile = name of ".c2" file (".c2" extension assumed)
    % oFile = output file name
	% f = frequency vs time (in Hz)
    % t = time points
    % output is over a 120-second time interval, with 111-second frame
    % toffset = random prefix, in seconds, in [0,9]
    % oSps =  output file sampling frequency
    % upSampling = up sampling factor
    %
    % 
    %%% read baseband signal from ".c2" file
    %
    fileID = fopen([iFile '.c2'],'r');
    % read file name (14 characters)
    fname=fread(fileID,[1 14],'char');
    % read WSPR type (integer) 2="2 minute mode"
    type=fread(fileID,[1 1],'int'); 
    % read frequency (double)
    dfreq=fread(fileID,[1 1],'double');
    % read data (float)
    buffer=fread(fileID,[2 45000],'float');
    % close the input file
    fclose(fileID);
    % in-phase
    idat=buffer(1,:);
    % quadrature
    qdat=buffer(2,:);
    %
    %%% up conversion
    %
    % up sampling factor
    upSampling = oSps / 375;
    % resample the baseband signal for the ".wav" format
    x=resample(complex(idat,qdat),upSampling,1);
    % number of time points, over a 120-second interval
    xl = oSps*120;
    % generate time points
    T = 120; % period (seconds)
    dt = T/xl; % sample period (second)
    t = 0:dt:T-dt;
    % modulation
    y = x.*exp(-1i*(2*pi*f.*t));
    % normalize in the [-1 1] range
    y = y/(max(abs(min(y)),max(y)));
    % add a random noise prefix during time offeset
    r=rand(1,round(toffset*oSps));
    y =[r(:);y];
    % write the wave file
    audiowrite(oFile,real(y),oSps,'BitsPerSample',BitsPerSample);
end


