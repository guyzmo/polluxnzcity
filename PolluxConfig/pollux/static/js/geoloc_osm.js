//http://nominatim.openstreetmap.org/search/?q=19%20galerie%20feydeau,%2075002,%20Paris,%20France&format=json

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
                        $('#osm_view').click(function() {window.open('http://www.openstreetmap.org/?lat='+data[0].lat+'&lon='+data[0].lon+'&zoom=16&layers=M')})
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
        $('#osm_view').click(function() {window.open('http://www.openstreetmap.org/?lat='+latitude+'&lon='+longitude+'&zoom=16&layers=M')})
}
});
