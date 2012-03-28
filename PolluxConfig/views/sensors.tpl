<%inherit file="base.tpl"/>
<h1>Pollux nz city configuration utility</h1>
<form class="form-horizontal" method="post">

<fieldset>
   <legend>Sensor List</legend>
%for name, sensorl in sensors.iteritems():
<div class="control-group">
   <label class="control-label" for="sensor_addr">Sensor module at address</label>
   <div class="controls">
   <input type="text" class="input-xlarge" id="sensor_addr" name="sensor_addr" value="${name}">
   <!--<p class="help-block">Supporting help text</p>-->
   </div>
   </div>

<table class="table table-striped table-bordered">
   	<thead>
   		<tr>
   			<th width="80px">Activated</th>
   			<th>Sensor</th>
   		</tr>
   	</thead>
	<tbody>
		<tr>
%for s in sensorl:
			<td><input type="checkbox" name="${s["address"]}_${s["register"]}" id="${s["address"]}_${s["register"]}"/></td>
			<td><label for="${s["address"]}_${s["register"]}">${s["name"]}
%if "unit" in s.keys():
			(${s["unit"]})
%endif
			</label></td>
		<tr>
%endfor
   	</tbody>
   </table>

%endfor

</fieldset>
	<center><input type="submit" class="btn btn-primary btn-large"/></center>
</form>