
%Author Ahmad Traboulsi
%A Component that allows the visualization of the volume of an audio signal
%being played

classdef VolumeMeter < matlab.ui.componentcontainer.ComponentContainer
    
 properties(Access = public)
             Value {validateattributes(Value,{'double'}, ... 
            {'<=',1,'>=',0,'size',[1 3]})} = [1 0 0]; 
             rect=0;
 end
 
 	
    events (HasCallbackProperty, NotifyAccess = protected) 
        ValueChanged % ValueChangedFcn will be the generated callback property 
    end
    
 properties(Access = private)
        Grid matlab.ui.container.GridLayout 
        Button matlab.ui.control.Button 
        EditField matlab.ui.control.EditField 
%        VolumeMeterFig matlab.ui.Figure
        VolumeMeterAxes matlab.ui.control.UIAxes
 end
 
 
 methods(Access = protected)
     
     function setup(obj, grid)
         
         if ~exist('grid','var')
         % Create Grid Layout object to manage building blocks 
         obj.Grid = uigridlayout(obj,[1,2]); 
         else
             obj.Grid = grid;
         end
        

     % Text Field for value display and Button to launch uisetcolor 
             obj.EditField = uieditfield(obj.Grid,'Editable',false,... 
                'HorizontalAlignment','center'); 
           obj.Button = uibutton(obj.Grid,'Text',char(9998), ... 
                'ButtonPushedFcn',@(o,e) obj.getColorFromUser()); 
            %obj.VolumeMeterFig = uiaxes(obj.Grid,'Position',[10 10 400 300], 'xTick', [], 'yTick',[1,2,3,4,5,6,7,8,9,10], 'YLim',[0,10],'YColorMode','manual', 'YColor','black','visible','on','Color','white','Clipping','off');
            %obj.VolumeMeterFig = uifigure(obj.Grid,'Position',[10 10 400 300]);
            %obj.Grid.RowHeight = {'fit',1,1,'100px'};
            %obj.Grid.ColumnWidth = {1,'400px'};

            obj.VolumeMeterAxes = uiaxes(obj.Grid,'Position',[10 10 400 300], 'xTick', [], 'yTick',[1,2,3,4,5,6,7,8,9,10], 'YLim',[0,10],'YColorMode','manual', 'YColor','black','visible','on','Color','white','Clipping','off');
            obj.VolumeMeterAxes.Layout.Row=1;
            obj.VolumeMeterAxes.Layout.Column=10;
            
            obj.VolumeMeterAxes.Toolbar.Visible='off';
            obj.VolumeMeterAxes.Interactions=[];
            obj.rect =rectangle('Position',[10 10 300 200],'Curvature',0,'Parent',obj.VolumeMeterAxes,'FaceColor',[0 .5 .5],'EdgeColor','none');


     end
     
     function update(obj)
            pause(5)
            delete(obj.rect)
            obj.rect=rectangle('Position',[0 0 2 5],'Curvature',0,'Parent',obj.VolumeMeterAxes,'FaceColor',[0 .7 .7],'EdgeColor','none'); 
     end
     
 end
 
end


    