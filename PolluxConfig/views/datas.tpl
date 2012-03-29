<%inherit file="base.tpl"/>
<%def name="extra_css()">
<script type="text/javascript" src="/js/dygraph.min.js"></script>
<script>
  $(document).ready(function () {
  	g = new Dygraph(
	  document.getElementById("graphdiv"),
		"/data/data.csv",{
			legend: "always",
			labelsDivStyles: {textAlign: "right"},
			showRangeSelector: true
		});
});
</script>
</%def>
<div id="graphdiv"  style="width:800px; height:320px;"></div>
<br />
<br />