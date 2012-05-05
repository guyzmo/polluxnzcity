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
%for key, value in config.iteritems():
            <div class="control-group">
                <label class="control-label" for="${key}">${key}</label>
                <div class="controls">
                    <input type="text" class="input-xlarge" id="${key}" name="${key}" value="${value}">
                    <!--<p class="help-block">Supporting help text</p>-->
                </div>
            </div>
%endfor
        </fieldset>
		<center><input type="submit" class="btn btn-primary btn-large"/></center>
	</form>

    <form class="form-horizontal" method="post" action="/system/module/upload">
        <fieldset>
            <legend>Datastore plugin uploader</legend>
            <p> If you have a python plugin ready to upload, please enter the module's name below, and look for it.</p>
            <div class="control-group">
                <label class="control-label" for="name">Module's name</label>
                <div class="controls">
                    <input type="text" class="input-xlarge" name="name">
                </div>
            </div>
            <div class="control-group">
                <label class="control-label" for="name">Module's source (python2.7 compatible file)</label>
                <div class="controls">
                    <input type="file" class="input-xlarge" name="module">
                </div>
            </div>
            <center>
                <input type="submit" class="btn btn-primary btn-large"/>
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

