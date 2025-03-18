
import pandas as pd
import maidenhead as mh

file = './grid.csv'
dat = pd.read_csv(file)

#print(dat)

kml_header = '''
<kml xmlns="http://www.opengis.net/kml/2.2">
<Document>
<name>Flight Path</name>
<description>Flight Data for start time 2025-03-18T14:16:00Z, at site: 39.6463,280.0298, standard flight profile.</description>
<Style id="redPoly">
<LineStyle>
<color>7f0000ff</color>
<width>4</width>
</LineStyle>
<PolyStyle>
<color>ff00ff00</color>
</PolyStyle>
</Style>
<Placemark>
<name>Flight path</name>
<description>Ascent rate: 5.0, descent rate: 12.0, with burst at 30000.0m.</description>
<styleUrl>#redPoly</styleUrl>
<LineString>
<extrude>1</extrude>
<tesselate>1</tesselate>
<altitudeMode>absolute</altitudeMode>
<coordinates>
'''

kml_placemark = '''
{},{},{}
'''
kml_footer = '''
</coordinates>
</LineString></Placemark>
  </Document>
</kml>

'''


print(kml_header)
i = 1;
for grid in dat['grid']:
	i = i + 1
	lon,lat = mh.to_location(grid,center='True')
	print(kml_placemark.format(lat,lon,0))
print(kml_footer)






