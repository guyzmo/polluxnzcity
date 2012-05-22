//http://nominatim.openstreetmap.org/search/?q=19%20galerie%20feydeau,%2075002,%20Paris,%20France&format=json

var map ;
var position;
var mapnik;
var fromProjection;
var toProjection ;

var zoom = 18; 

$(document).ready(function() {
	map            = new OpenLayers.Map("basicMap");
    mapnik         = new OpenLayers.Layer.OSM();
    fromProjection = new OpenLayers.Projection("EPSG:4326");   // Transform from WGS 1984
    toProjection   = new OpenLayers.Projection("EPSG:900913"); // to Spherical Mercator Projection
	position       = new OpenLayers.LonLat($('#geo_longitude').val(),$('#geo_latitude').val()).transform( fromProjection, toProjection);
 
    map.addLayer(mapnik);
    map.setCenter(position, zoom );
});


var showError = function(t) {
    $('#osm_err_t').html(t);
    $('#osm_err').show();
}

var getCoordinates = function(address) {
    if (address == "") {
        $('#geo_latitude').val("");
        $('#geo_longitude').val("");
        $('#osm_view').text("");
        $('#geo_latitude').attr('readonly', false);
        $('#geo_longitude').attr('readonly', false);
        $('#osm_view').attr('readonly', true);
        $('#osm_view').click();
        $('#geoaddr_btn').val("Get coordinates");
    } else {
        $.ajax({
                url: '/geoloc/'+address,
                dataType: 'json',
                data: undefined,
                success: function(data) {
                    if (data.length == 1) {
                        $('#geo_latitude').val(data[0].lat);
                        $('#geo_longitude').val(data[0].lon);
                        $('#geo_latitude').attr('readonly', true);
                        $('#geo_longitude').attr('readonly', true);
                        $('#geoaddr_btn').val("Change/Reset");
                        $('#osm_view').attr('readonly', false);
						position = new OpenLayers.LonLat($('#geo_longitude').val(),$('#geo_latitude').val()).transform( fromProjection, toProjection);
						map.setCenter(position,zoom);
                    } else if (data.length == 0) {
                        showError("Address not found on Open Street Map.");
                    } else {
                        var addr_l = [];
                        for (v in data) {
                            addr_l.push("<li>"+data[v]['display_name']+"</li>");
                        }
                        showError("Please be more accurate on the address, we have found "+data.length+" addresses:<br /><ul>"+addr_l.join("\n")+"</ul>");
                    }
                }   
        });
    }
}

$(document).ready(function() {
    if ($('#geo_latitude').val() != "" && $('#geo_longitude').val() != "") {
        $('#geo_latitude').attr('readonly', true);
        $('#geo_longitude').attr('readonly', true);
        $('#geoaddr_btn').val("Change/Reset");
        $('#osm_view').attr('readonly', false);
		/*$('#osm_view').click(function() {window.open('http://www.openstreetmap.org/?lat='+$('#geo_latitude').val()+'&lon='+$('#geo_longitude').val()+'&zoom=16&layers=M')})*/
}
});
