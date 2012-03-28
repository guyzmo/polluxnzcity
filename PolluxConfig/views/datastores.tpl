<%inherit file="base.tpl"/>
<h1>Pollux nz city configuration utility</h1>
%if welldone:
<br />
<div class="alert alert-success">Configuration successfully updated<a class="close" data-dismiss="alert" href="#">&times;</a></div>
%endif
<form class="form-horizontal" method="post">
<fieldset>
   <legend>Datastores configuration</legend>
%for name, datastore in datastores.iteritems():
<fieldset>
<legend>${name}</a></legend>
%for key, value in datastore.iteritems():
<div class="control-group">
   <label class="control-label" for="${name}_${key}">${key}</label>
   <div class="controls">
%if key == "activated":
<%
checked = "checked" if value else "" 
%>
   <input type="checkbox" name="${name}_${key}" id="${name}_${key}" ${checked} />
%else:
   <input type="text" class="input-xlarge" id="${name}_${key}" name="${name}_${key}" value="${value}">
%endif
   <!--<p class="help-block">Supporting help text</p>-->
   </div>
   </div>
%endfor
</fieldset>
%endfor

</fieldset>
	<center><input type="submit" class="btn btn-primary btn-large"/></center>
</form>