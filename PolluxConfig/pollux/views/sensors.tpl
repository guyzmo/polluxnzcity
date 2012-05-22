<%inherit file="base.tpl"/>

<h1>Pollux nz city configuration utility</h1>
%if welldone:
<br />
<div class="alert alert-success">Configuration successfully updated<a class="close" data-dismiss="alert" href="#">&times;</a></div>
%else:
<div id="osm_err" class="alert alert-error hide fade in" data-alert="alert"><a class="close" data-dismiss="alert" href="#">&times;</a><span id="osm_err_t"></span></div>
%endif
<form class="form-horizontal" method="post">

<fieldset>
   <legend>Sensor List</legend>
%for name, sensorsl in sensors.get_sensors().iteritems():
    <div id="accordion_${name}" class="accordion">
        <div class="accordion-group">
            <div class="accordion-heading">
                <div class="control-group">
                    <label class="control-label" for="sensor_addr"><a data-toggle="collapse" data-parent="#accordion_${name}" href="#collapse_${name}">Sensor's address</a></label>
                    <div class="controls">
                        <input type="text" class="input-medium" id="sensor_addr" name="sensor_addr" value="${name}">
                        <input type="hidden" id="sensor_addr_old" name="sensor_addr_old" value="${name}">
                    </div>
                </div>
            </div>
            <div id="collapse_${name}" classe="accordion-body collapse">
                <div class="accordion-inner">
                    <table class="table table-striped table-bordered">
                        <thead>
                            <tr>
                                <th width="80px">Activated</th>
                                <th>Sensor</th>
                            </tr>
                        </thead>
                        <tbody>
<%
sensorsl_names = [s["name"] for s in sensorsl]
sensorsd_activ = dict([(s["name"],s["activated"]) for s in sensorsl])
%>
%for s in sensors.get_sensors_list():
                            <tr>
<%
checked = "checked" if s["name"] in sensorsl_names and sensorsd_activ[s["name"]] else "" 
%>
                                <td><input type="checkbox" name="${s["address"]}_${s["register"]}" id="${s["address"]}_${s["register"]}" ${checked} /></td>
                                <td><label for="${s["address"]}_${s["register"]}">${s["name"]}
%if "unit" in s.keys():
                                        (${s["unit"]})
%endif
                                    </label>
                                </td>
                            </tr>
%endfor
                        </tbody>
                    </table>
                </div>
            </div>
            <!-- <input type="button" class="btn btn-medium" value="Delete sensor" disabled /><br /> -->
        </div>
    </div>
%endfor
    <!-- <input type="button" class="btn btn-medium" value="Add a sensor" disabled /><br />-->

	<center><input type="submit" class="btn btn-primary btn-large"/></center>
</fieldset>
</form>
