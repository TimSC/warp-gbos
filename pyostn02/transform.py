from OSGB import *
from OSTN02 import *

def TransformOsgbToWgs84(x0,y0,z0):
	ret = OSGB36_to_ETRS89(x0,y0,z0)
	if ret == None: 
		#print "OSTN02 did not return result at",x0,y0,z0
		return None
	(x, y, z) = ret
	(gla, glo) = grid_to_ll(x, y, 'WGS84')
	#print "result",gla, glo, z
	return (gla, glo, z)

