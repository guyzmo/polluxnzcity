<%inherit file="base.tpl"/>
<%def name="extra_css()">
<script type="text/javascript" src="/js/dygraph.min.js"></script>
<script>
function change(el) {
  chart.setVisibility(el.id, el.checked);
}

  $(document).ready(function () {
  	chart = new Dygraph(
	  document.getElementById("graphdiv"),
		"/data/csv",{
			legend: "always",
			labelsDivStyles: {textAlign: "right"},
			showRangeSelector: true,
            visibility: [false, false, false, true]
		});
});
</script>
</%def>
<div id="graphdiv"  style="width:800px; height:320px;"></div>
<br />
<form class="form-inline">
  <label for="0"> Noise                <input type="checkbox" id="0" onClick="change(this)"></label>
  <label for="1"> Internal Temperature <input type="checkbox" id="1" checked onClick="change(this)"></label> 
  <label for="2"> Dust                 <input type="checkbox" id="2" onClick="change(this)"></label>
  <label for="3"> Temperature          <input type="checkbox" id="3" onClick="change(this)"></label> 
</form>
<br />
