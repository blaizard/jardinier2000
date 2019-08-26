#include "types.h"

node::Array<node::Data, 4> node::dataSupported(
	node::Data{
		.m_type = node::DataType::HUMIDITY,
		.m_name = "humidity"
	},
	node::Data{
		.m_type = node::DataType::LUMINOSITY,
		.m_name = "luminosity"
	},
	node::Data{
		.m_type = node::DataType::MOISTURE,
		.m_name = "moisture"
	},
	node::Data{
		.m_type = node::DataType::TEMPERATURE,
		.m_name = "temperature"
	}
);
