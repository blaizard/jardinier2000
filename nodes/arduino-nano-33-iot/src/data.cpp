#include "data.h"

node::Array<node::data::Data, 4> node::data::supported(
	node::data::Data{
		.m_type = node::data::Type::HUMIDITY,
		.m_name = "humidity"
	},	
	node::data::Data{
		.m_type = node::data::Type::LUMINOSITY,
		.m_name = "luminosity"
	},
	node::data::Data{
		.m_type = node::data::Type::MOISTURE,
		.m_name = "moisture"
	},
	node::data::Data{
		.m_type = node::data::Type::TEMPERATURE,
		.m_name = "temperature"
	}
);
