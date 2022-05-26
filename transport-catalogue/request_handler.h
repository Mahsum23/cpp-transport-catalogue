#pragma once

#include <iostream>

#include "transport_catalogue.h"
#include "json_reader.h"

namespace transport_catalogue
{
	class RequestHandler
	{
	public:
		RequestHandler(const catalogue::TransportCatalogue& catalogue, const reader::JsonReader& reader, const renderer::MapRenderer& renderer);
		json::Node ProcessInfoAsJson() const;
		
	private:
		const catalogue::TransportCatalogue& catalogue_;
		const reader::JsonReader& reader_;
		const renderer::MapRenderer& renderer_;

	};
}