<%inherit file="base.tpl"/>
<h1>Pollux nz city configuration utility</h1>
%if welldone:
<br />
<div class="alert fade in alert-success">${message}<a class="close  fade in" data-dismiss="alert" href="#">&times;</a></div>
%endif
%if failed:
<br />
<div class="alert alert-error fade in">${message}<a class="close" data-dismiss="alert" href="#">&times;</a></div>
%endif
<form class="form-horizontal" method="post">
        <fieldset>
            <legend>General configuration</legend>
%for key, value in config.get_configuration().iteritems():
            <div class="control-group">
                <label class="control-label" for="${key}">${key}</label>
                <div class="controls">
                    <input type="text" class="input-xlarge" id="${key}" name="${key}" value="${value}">
                    <!--<p class="help-block">Supporting help text</p>-->
                </div>
            </div>
%endfor
        </fieldset>
		<center><input type="submit" class="btn btn-primary btn-large" value="Save" /></center>
	</form>

    <form class="form-horizontal" method="post" action="/system/module/upload" enctype='multipart/form-data'>
        <fieldset>
            <legend>Datastore plugin uploader</legend>
            <p> If you have a python plugin's source code ready to upload, 
            browse to it in the field below. The source code shall be python 2.7 compliant,
            and it shall contain the following symbols:
            <ul>
            <li>NAME : name of the module</li>
            <li>DESC : description of the module</li>
            <li>DEFAULT_CONFIG : default configuration values for the module</li>
            <li>push_to_datastore(config,values) : method to process the values, given the config</li>
            </ul></p>
            <div class="control-group">
                <label class="control-label" for="name">Module's source</label>
                <div class="controls">
                    <input type="file" class="input-xlarge" name="module">
                </div>
            </div>
            <center>
                <input type="submit" class="btn btn-primary btn-large" value="Upload" />
            </center>
        </fieldset>
    </form>

    <form class="form-horizontal" method="post" action="/system/module/delete" enctype='multipart/form-data'>
        <fieldset>
            <legend>Datastore plugin manager</legend>
            <table class="table table-striped table-bordered">
                <thead>
                    <tr>
                        <th width="80px">Selected</th>
                        <th>Name</th>
                        <th>Filename</th>
                        <th>Description</th>
                    </tr>
                </thead>
                <tbody>
% for name, filename, desc in config.list_plugins():
                    <tr>
                        <td><input type="checkbox" name="${filename}" id="${filename}"/></td>
                        <td><label for="${filename}">${name}</label></td> <!-- name -->
                        <td><label for="${filename}">${filename}</label></td> <!-- filename -->
                        <td><label for="${filename}">${desc}</label></td> <!-- desc -->
                    </tr>
% endfor
                </tbody>
            </table>

            <center>
                <input type="submit" class="btn btn-primary btn-large" value="Remove selected"/>
            </center>
        </fieldset>
    </form>

    <fieldset>
        <legend>System commands</legend>
        <center>
            <div class="control-group">
                <div class="controls">
                    <input type="button" class="btn btn-secondary btn-large" value="Reload configuration" onclick="location.href='/config/reload';" />
                    <input type="button" class="btn btn-secondary btn-large" value="Update sensors list"  onclick="location.href='/sensors/reload';" />
                    <input type="button" class="btn btn-secondary btn-large" value="Restart gateway service"  onclick="location.href='/system/restart';" />
                    <input type="button" class="btn btn-secondary btn-large" value="View logs"  onclick="location.href='/system/logs';" />
                </div>
            </div>
        </center>
    </fieldset>

