<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>Pollux Nz City :: ${title}</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="description" content="">
    <meta name="author" content="">

	<script type="text/javascript" src="/js/jquery-1.7.2.min.js"></script>
	<script type="text/javascript" src="/js/bootstrap.min.js"></script>
	<script src="/js/bootstrap-alert.js"></script>
	<script type="text/javascript" src="/js/custom.js"></script>
    ${self.extra_js()}

    <link href="/css/bootstrap.min.css" rel="stylesheet">
    <style>body { padding-top: 60px; } </style>
    <link href="/css/bootstrap-responsive.min.css" rel="stylesheet">
    ${self.extra_css()}

    <!-- Le HTML5 shim, for IE6-8 support of HTML5 elements -->
    <!--[if lt IE 9]>
      <script src="http://html5shim.googlecode.com/svn/trunk/html5.js"></script>
    <![endif]-->

    <!-- Le fav and touch icons
    <link rel="shortcut icon" href="../assets/ico/favicon.ico">
    <link rel="apple-touch-icon-precomposed" sizes="114x114" href="../assets/ico/apple-touch-icon-114-precomposed.png">
    <link rel="apple-touch-icon-precomposed" sizes="72x72" href="../assets/ico/apple-touch-icon-72-precomposed.png">
    <link rel="apple-touch-icon-precomposed" href="../assets/ico/apple-touch-icon-57-precomposed.png">  -->
  </head>

  <body>

<div class="navbar navbar-fixed-top">
  <div class="navbar-inner">
    <div class="container">
      <a class="btn btn-navbar" data-toggle="collapse" data-target=".nav-collapse">
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
      </a>
      <a class="brand" href="/">Pollux'NZ City</a>
      <div class="nav-collapse">
        <ul class="nav">
          <li><a href="/">Home</a></li>
          <li><a href="/datas/">MyDatas</a></li>
          <li><a href="/sensors/">Sensors</a></li>
          <li><a href="/datastores/">Datastores</a></li>
          <li><a href="/advanced/">Advanced</a></li>
        </ul>
      </div><!--/.nav-collapse -->
    </div>
  </div>
</div>

<div class="container">
	${self.body()}
<footer>
  <p>PolluxNzCity™ By <a href="http://ckab.net">CKAB</a>© 2012</p>
</footer>
</div> <!-- /container -->
</body>
</html>
<%def name="extra_css()"></%def>
<%def name="extra_js()"></%def>
