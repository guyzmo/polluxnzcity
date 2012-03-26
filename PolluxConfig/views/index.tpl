<html>
    <head>
    <script type="text/javascript" src="js/jquery-1.7.2.min.js">
    </script>
    </head>
    <body>
    <script>

// pffff... javascript sucks...

//    $(document).ready(function() {
//        var getData = $.ajax({url: '/sensor/list',
//                              dataType: 'json',
//                              data: undefined,
//                              success: function(data) {
//                                    var sensors_data = $.ajax({url: '/sensor/'+data,
//                                                                dataType: 'json',
//                                                                data2: undefined,
//                                                                async: false,
//                                                                success: function(data2) {
//                                                                    sensor_data = data2;
//                                                                }}).responseText;
//
//                                    $.each(eval(sensors_data), function(idx, val) { 
//                                    content = data + ": "
//                                        content = content +"<a href='/sensor/"+data+"/"+val+"'>"
//                                        content = content + val
//                                        content = content + "</a><br />"
//                                        $('#sensor_list').prepend(content);
//                                    })
//                              }});
//                            }
//    );
    </script>
        <h1>Pollux nz city configuration utility</h1>

<h2>General configuration</h2>

%for key, value in config.iteritems():
<p>{{key}} : <input type="text" value="{{value}}"></p>
%end

<h2>Datastores configuration</h2>

%for name, datastore in datastores.iteritems():
<p>
<h3>{{name}}</h3>
%   for key, value in datastore.iteritems():
<p>{{key}} : <input type="text" value="{{value}}"></p>
%   end
</p>
%end

<h2>Sensor List</h2>

%for name, sensorl in sensors.iteritems():
Sensor module at address: <input type="text" value="{{name}}">
<table>
    <tr>
        <td>name</td><td>unit</td><td>address</td><td>register</td>
    </tr>
%   for s in sensorl:
    <tr>
        <td><input type="text" value="{{s["name"]}}"></td>
%if "unit" in s.keys():
        <td><input type="text" value="{{s["unit"]}}"></td>
%else:
        <td><input type="text" value=""></td>
%end
        <td><input type="text" value="{{s["address"]}}"></td>
        <td><input type="text" value="{{s["register"]}}"></td>
    </tr>
%   end
</table>
</p>
%end


    </body>

</html>
