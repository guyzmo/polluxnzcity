<%inherit file="base.tpl"/>
<h1>Pollux nz city configuration utility</h1>
<form class="form-horizontal" method="post">
<fieldset>
   <legend>Datastores configuration</legend>
%for name, datastore in datastores.iteritems():
<fieldset>
<legend>${name} <a class="toggle" href="#" onclick="toggle('${name}')">[+]</a></legend>
<div id="${name}" style="display:none"><div class="control-group">
   <label class="control-label" for="${name}_activated">activated</label>
   <div class="controls">
   <input type="checkbox" name="${name}_activated" id="${name}_activated" />
   <!--<p class="help-block">Supporting help text</p>-->
   </div>
   </div>
%for key, value in datastore.iteritems():
<div class="control-group">
   <label class="control-label" for="${key}">${key}</label>
   <div class="controls">
   <input type="text" class="input-xlarge" id="${key}" name="${key}" value="${value}">
   <!--<p class="help-block">Supporting help text</p>-->
   </div>
   </div>
%endfor
</div>
</fieldset>
%endfor

</fieldset>
	<center><input type="submit" class="btn btn-primary btn-large"/></center>
</form>