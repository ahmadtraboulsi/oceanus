%function [result, cycles, output, gamma, RMS, SixHzSNNR, best_lag]
function allresults = decodesignal(x, mode, max_cycles, searchType, MINRMS, MINSNR)
%n=45000; %number of samples
%   searchtype =    1 (searching for candidate signals, decode strongest),
%                   2 (frequencies provided in FREQ, decode all)
%                   3 (frequencies provided in FREQ, combine energy)
n=length(x); %number of samples in x
fs= 375; %sampling frequency
spsymbol = [128, 256, 512, 1024, 2048, 2560];
interval = [1 2 4 8 16 20];
windowSize = 375*interval(mode)*60;
popWindowSize = (windowSize/375 - ceil(162 * spsymbol(mode) /375)) * 375;



[a,b]= size(x);
allresults=[];
if a ==1
        successFlag= 0;

    z=x;
    % originally was for delay=0:1:popWindowSize - changed on 27/08/2022 to 100
    for delay=0:100:popWindowSize
        % add delay (in samples) and extend to 60 seconds
        % delay = 1125;
        zz = [z(delay+1:end) zeros(1,delay)];
        %disp(delay);
        %whos zz
        % in-phase
        inph = real(zz);
        % quadrature
        quad = imag(zz);
        % constraint
        K=32;
        % message with K-1 zeros; Corresponds to payload "VE3EMB FN25 30"
        m = [1 1 0 1 0 1 0 0 0 0 1 0 1 1 0 0 0 1 1 1 0 0 1 1 1 1 1 0 1 0 1 1 0 0 1 1 1 0 1 0 0 1 1 1 0 1 1 1 1 0 zeros(1,K-1)];
        allresults = framesearch(...
            inph, quad, length(inph), fs, 0, 16.25, MINSNR, spsymbol(mode), interval(mode),max_cycles, searchType);
        
        [all_a,all_b] = size(allresults);
        
        for i=1:size(allresults,1)
            % [ result, cycles, output, gamma, RMS, SixHzSNR ]
            if ~allresults(i,1)
                fprintf("Decoding success; cycles=%d; gamma=%d; RMS=%2.2f; 6 Hz SNR=%2.2f db!\n",...
                    allresults(i,2), allresults(i,84), allresults(i,85), allresults(i,86));
                if isequal(allresults(i,3:83),m)
                    fprintf("With no errors!\n");
                    successFlag=1;
                    break;
                else
                    fprintf("With %d errors!\n", sum(allresults(i,3:83)~=m));
                end
            else
                if all_b == 86
                fprintf("Decoding failure; cycles=%d; gamma=%d; RMS=%2.2f!\n",...
                    allresults(i,2), allresults(i,84), allresults(i,85));
                else
                fprintf("Decoding failure; cycles=%d; gamma=%d; RMS=%2.2f!\n",...
                    allresults(i,2), allresults(i,4), allresults(i,5));
                end
            end
        end
        
        if successFlag ==1
            break;
        end
%         if ~result
%             %fprintf("Decoding success; cycles=%d; gamma=%d; RMS=%2.2f; 6 Hz SNR=%2.2f db!\n",cycles, gamma, RMS, SixHzSNR);
%             %present(output)
%             %         if isequal(output,m)
%             %             fprintf("With no errors!\n");
%             %         else
%             %             fprintf("With %d errors!\n", sum(output~=m));
%             %         end
%             %disp(delay);
%             break;
%         end
    end
elseif a > 1
    
    
    successFlag= 0;
    for delay=0:100:popWindowSize
        inph = [];
        quad = [];
        for k=1:a
            z=x(k,:);
            % add delay (in samples) and extend to 60 seconds
            % delay = 1125;
            zz = [z(delay+1:end) zeros(1,delay)];
            %disp(delay);
            %whos zz
            % in-phase
            inph =[ inph ; real(zz)];
            % quadrature
            quad = [quad ; imag(zz)];
            % constraint
            K=32;
            % message with K-1 zeros; Corresponds to payload "VE3EMB FN25 30"
            %m = [1 1 0 1 0 1 0 0 0 0 1 0 1 1 0 0 0 1 1 1 0 0 1 1 1 1 1 0 1 0 1 1 0 0 1 1 1 0 1 0 0 1 1 1 0 1 1 1 1 0 zeros(1,K-1)];
        end
        m = [1 1 0 1 0 1 0 0 0 0 1 0 1 1 0 0 0 1 1 1 0 0 1 1 1 1 1 0 1 0 1 1 0 0 1 1 1 0 1 0 0 1 1 1 0 1 1 1 1 0 zeros(1,K-1)]; 
        try
        allresults = framesearch(...
            inph, quad, length(inph), fs, 0, MINRMS, MINSNR, spsymbol(mode), interval(mode),max_cycles, searchType);
        
        [all_a,all_b] = size(allresults);
        
        for i=1:size(allresults,1)
            % [ result, cycles, output, gamma, RMS, SixHzSNR ]
            if ~allresults(i,1)
                fprintf("Decoding success; cycles=%d; gamma=%d; RMS=%2.2f; 6 Hz SNR=%2.2f db!\n",...
                    allresults(i,2), allresults(i,84), allresults(i,85), allresults(i,86));
                if isequal(allresults(i,3:83),m)
                    fprintf("With no errors!\n");
                    successFlag=1;
                    break;
                else
                    fprintf("With %d errors!\n", sum(allresults(i,3:83)~=m));
                end
            else
                if all_b == 86
                fprintf("Decoding failure; cycles=%d; gamma=%d; RMS=%2.2f!\n",...
                    allresults(i,2), allresults(i,84), allresults(i,85));
                else
                fprintf("Decoding failure; cycles=%d; gamma=%d; RMS=%2.2f!\n",...
                    allresults(i,2), allresults(i,4), allresults(i,5));
                end
            end
        end
        
        catch EX
           %
           disp(EX)
           result = 1; cycles = 0; output = []; gamma = 0; SixHzSNR = 1;
           allresults = [result, cycles, output, gamma, -1, SixHzSNR];
            
        end
        
        if successFlag ==1
            break;
        end
        
        %if ~allresults(1)
        %fprintf("Decoding success; cycles=%d; gamma=%d; RMS=%2.2f; 6 Hz SNR=%2.2f db!\n",cycles, gamma, RMS, SixHzSNR);
        %present(output)
        %         if isequal(output,m)
        %             fprintf("With no errors!\n");
        %         else
        %             fprintf("With %d errors!\n", sum(output~=m));
        %         end
        %disp(delay);
        %break;
        %end
        
    end
    
end
%[result, cycles, output, gamma, RMS, SixHzSNNR, best_lag]  = framesearch(...
%    I, Q, n,fs, 0, 16.25, spsymbol(mode), interval(mode), max_cycles);
end


