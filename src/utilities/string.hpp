//
// created by tift0 on 09.03.2025.
//

#pragma once

namespace string {
	inline void int_to_str(int value, char* buffer) {
		auto rev_str = []( char* str, int length ) -> void {
			int start{};
			int end = length - 1;
			while (start < end) {
				char temp = str[start];
				str[start] = str[end];
				str[end] = temp;
				start++;
				end--;
			}
		};

		int i{},
			is_negative{};

		if (value < 0) {
			is_negative = 1;
			value = -value;
		}

		do {
			buffer[i++] = value % 10 + '0';
			value /= 10;
		} while (value > 0);

		if (is_negative)
			buffer[i++] = '-';

		buffer[i] = '\0';

		rev_str(buffer, i);
	}

	inline char* cat_str(const char* str1, const char* str2) {
		size_t len1 = strlen(str1);
		size_t len2 = strlen(str2);

		char* result = (char*)malloc(len1 + len2 + 1);

		if (result == NULL)
			return NULL;

		strcpy(result, str1);
		strcat(result, str2);

		return result;
	}
}