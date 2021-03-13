function [insig, qdsig] = wspr(message, snr)
if not(libisloaded('wsprsim'))
loadlibrary('wsprsim')
end
insig = libpointer('doublePtr', zeros(1,45000));
qdsig = libpointer('doublePtr', zeros(1,45000));
calllib('wsprsim', 'wsprfunc','VE3EMB FN25 30', 30, insig, qdsig)
%t=(1:45000);
%plot(t,insig.val);
%figure 
%plot(t,qdsig.val);
end