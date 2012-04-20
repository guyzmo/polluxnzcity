<%inherit file="base.tpl"/>
<h1>Pollux nz city log viewer</h1>
%if welldone:
<br />
<div class="alert fade in alert-success">${message}<a class="close  fade in" data-dismiss="alert" href="#">&times;</a></div>
%endif
%if failed:
<br />
<div class="alert alert-error hide fade in" data-alert="alert">${message}<a class="close" data-dismiss="alert" href="#">&times;</a></div>
%endif


%for l in logs:
${l}</br>
%endfor

