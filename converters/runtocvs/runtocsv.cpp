#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include <string.h>

#include "..\common\channel_data.h"
#include "..\common\csv_data.h"

void process_file(char * input_file_name)
{
	char * output_file_name = _strdup(input_file_name);
	char * extension = strrchr(output_file_name, '.');

	if (extension != NULL && strcmp(extension,".run")==0)
	{
		extension[1] = 'c';
		extension[2] = 's';
		extension[3] = 'v';

		FILE * output_file = NULL;

		if(fopen_s(&output_file, output_file_name, "w")==0)
		{
			c_channel_data_decoder data_decoder;

			data_decoder.begin(input_file_name);

			c_channel_data * channel_data = data_decoder.get_channel_data();

			while(channel_data!=NULL && channel_data->get_channel() != e_channel_time_stamp)
			{
				delete channel_data;
				channel_data = data_decoder.get_channel_data();
			}

			if (channel_data!=NULL) c_csv_data::write_header(output_file);

			bool position_offset_set = false;
			double position_offset_x = 0.0;
			double position_offset_y = 0.0;
			double position_offset_latitude = 0.0;
			double position_offset_longitude = 0.0;

			while(channel_data!=NULL)
			{
				assert(channel_data->get_channel()==e_channel_time_stamp);

				c_csv_data csv_data;

				channel_data->get_csv_data(&csv_data);

				while(1)
				{
					delete channel_data;
					channel_data = data_decoder.get_channel_data();

					if (channel_data==NULL || channel_data->get_channel()==e_channel_time_stamp)
						break;


                    channel_data->get_csv_data(&csv_data);
				}

				if (!position_offset_set)
				{
					position_offset_latitude = -csv_data.m_gps_latitude_position;
					position_offset_longitude = -csv_data.m_gps_longitude_position;
					position_offset_x = -csv_data.m_gps_x;
					position_offset_y = -csv_data.m_gps_y;
					position_offset_set = true;
				}

				csv_data.m_gps_latitude_position += position_offset_latitude;
				csv_data.m_gps_longitude_position += position_offset_longitude;
				csv_data.m_gps_x += position_offset_x;
				csv_data.m_gps_y += position_offset_y;

				csv_data.write(output_file);

			}
			data_decoder.end();

			fclose(output_file);
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	for (int i=1; i < argc; i++)
	{
		char * input_file_name = argv[i];

		process_file(input_file_name);
	}

	return 0;
}
