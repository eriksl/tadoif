#include "tadoif.h"

#include <iostream>
#include <string>
#include <boost/format.hpp>

int main(int argc, const char **argv)
{
	try
	{
		TadoIf tado_if;
		TadoIf::Data data;
		int zone_nr;
		char timestring[64];
		const struct tm *tm;
		time_t timestamp;

		data = tado_if.fetch(argc, argv);
		zone_nr = 0;

		for(const auto &zone : data.zones)
		{
			timestamp = zone.time;
			tm = localtime(&timestamp);
			strftime(timestring, sizeof(timestring), "%Y-%m-%d %H:%M:%S", tm);

			std::cout << boost::format("%s %s %s %s %s %s %s %s\n") %
					"zone #" % "id" % "name" % "active" % "power" % "temperature" % "humidity" % "time";
			std::cout << boost::format("%d %s %s %s %f %f %f %s\n") %
					zone_nr %
					zone.id %
					zone.name %
					(zone.active ? "active" : "inactive") %
					(zone.power * 100) %
					zone.temperature %
					zone.humidity %
					timestring;

			zone_nr++;
		}
	}
	catch(const TadoIf::Exception &e)
	{
		std::cerr << e.what() << std::endl;
		return(-1);
	}
	catch(const std::exception &e)
	{
		std::cerr << "standard exception: " << e.what() << std::endl;
		return(-1);
	}
	catch(...)
	{
		std::cerr << "unknown exception" << std::endl;
		return(-1);
	}

	return(0);
}
