<%inherit file="base.tpl"/>

<%block name="extra_js">
    <script src="/js/geoloc_osm.js"></script>
</%block>

<h1>Pollux nz city configuration utility</h1>
%if welldone:
<br />
<div class="alert alert-success">Configuration successfully updated<a class="close" data-dismiss="alert" href="#">&times;</a></div>
%else:
<div id="osm_err" class="alert alert-error hide fade in" data-alert="alert"><a class="close" data-dismiss="alert" href="#">&times;</a><span id="osm_err_t"></span></div>
%endif

<form class="form-horizontal" method="post">
<fieldset>
    <legend>Geolocalisation</legend>
    <fieldset>
        <div class="control-group">
            <label class="control-label" for="longitude">Address</label>
            <div class="controls">
                <input type="text" class="input-xlarge" id="geo_address" name="geo_address" value="${geoloc["address"]}" />
                <input type="button" class="btn btn-medium" onClick="getCoordinates($('#geo_address').val())" id="geoaddr_btn" value="Get coordinates" />
                <input type="button" class="btn btn-medium" id="osm_view" value="Lookup on OSM" readonly=true /><br />
            </div>
            <label class="control-label" for="geo_longitude">Longitude</label>
            <div class="controls">
                <input type="text" class="input-large" id="geo_longitude" name="geo_longitude" value="${geoloc["longitude"]}" /><br />
            </div>
            <label class="control-label" for="geo_latitude">Latitude</label>
            <div class="controls">
                <input type="text" class="input-large" id="geo_latitude" name="geo_latitude" value="${geoloc["latitude"]}" />
            </div>
        </div>
    </fieldset>

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
