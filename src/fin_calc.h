#pragma once

#include <vector>
#include <string>

class LongFloat {
public:
	LongFloat();
	LongFloat(std::string str);
	std::string to_string() const;
	LongFloat operator+(const LongFloat& second);
	LongFloat operator-(const LongFloat& second);
	bool operator==(const LongFloat& second)const;
	bool operator!=(const LongFloat& second)const;
	bool operator>(const LongFloat& second)const;
	bool operator<(const LongFloat& second)const;


private:
	static LongFloat Normalize(const LongFloat& f);
	static LongFloat SumTwoNumbersWnoSign(const LongFloat& one, const LongFloat& two);
	static LongFloat SubTwoNumbersWnoSign(const LongFloat& one, const LongFloat& two);
	bool is_negative_ = false;
	std::vector<int8_t> whole_part_;
	std::vector<int8_t> fractional_part_;
};