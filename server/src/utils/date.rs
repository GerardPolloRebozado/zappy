use std::fmt;

const SECONDS_IN_A_DAY: u64 = 86_400;

#[derive(Clone)]
pub struct Date {
    pub day: u8,
    pub month: u8,
    pub year: u16,
    pub hour: u8,
    pub minute: u8,
    pub second: u8,
    pub millisecond: u16,
}

impl Date {
    fn is_leap_year(year: u16) -> bool {
        (year.is_multiple_of(4) && !year.is_multiple_of(100)) || year.is_multiple_of(400)
    }

    pub fn now() -> Date {
        let current_time = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap();

        let total_seconds = current_time.as_secs();
        let millisecond = current_time.subsec_millis() as u16;
        let mut days_since_epoch = total_seconds / SECONDS_IN_A_DAY;
        let seconds_today = total_seconds % SECONDS_IN_A_DAY;

        let hour = (seconds_today / 3600) as u8;
        let minute = ((seconds_today % 3600) / 60) as u8;
        let second = (seconds_today % 60) as u8;

        let mut year = 1970;
        loop {
            let days_in_this_year = if Self::is_leap_year(year) { 366 } else { 365 };

            if days_since_epoch >= days_in_this_year {
                days_since_epoch -= days_in_this_year;
                year += 1;
            } else {
                break;
            }
        }

        let days_in_months = [
            31,
            if Self::is_leap_year(year) { 29 } else { 28 },
            31,
            30,
            31,
            30,
            31,
            31,
            30,
            31,
            30,
            31,
        ];

        let mut month = 0;
        while month < 12 {
            if days_since_epoch >= days_in_months[month] {
                days_since_epoch -= days_in_months[month];
                month += 1;
            } else {
                break;
            }
        }

        let day = (days_since_epoch + 1) as u8;

        Date {
            day,
            month: (month + 1) as u8,
            year,
            hour,
            minute,
            second,
            millisecond,
        }
    }

    pub fn to_timestamp(&self) -> u64 {
        let mut total_days = 0;
        for year in 1970..self.year {
            total_days += if Self::is_leap_year(year) { 366 } else { 365 };
        }

        let days_in_months = [
            31,
            if Self::is_leap_year(self.year) {
                29
            } else {
                28
            },
            31,
            30,
            31,
            30,
            31,
            31,
            30,
            31,
            30,
            31,
        ];

        for item in days_in_months.iter().take((self.month - 1) as usize) {
            total_days += item;
        }

        total_days += (self.day - 1) as u64;

        (total_days * SECONDS_IN_A_DAY
            + (self.hour as u64 * 3600)
            + (self.minute as u64 * 60)
            + self.second as u64)
            * 1000
            + self.millisecond as u64
    }

    pub fn from_string(string: &str) -> Option<Date> {
        let parts: Vec<&str> = string.split_whitespace().collect();
        if parts.len() != 2 {
            return None;
        }

        let date_parts: Vec<&str> = parts[0].split('/').collect();
        if date_parts.len() != 3 {
            return None;
        }

        let time_parts: Vec<&str> = parts[1].split(':').collect();
        if time_parts.len() != 2 {
            return None;
        }

        let day = date_parts[0].parse::<u8>().ok()?;
        let month = date_parts[1].parse::<u8>().ok()?;
        let year = date_parts[2].parse::<u16>().ok()?;
        let hour = time_parts[0].parse::<u8>().ok()?;
        let minute = time_parts[1].parse::<u8>().ok()?;

        Some(Date {
            day,
            month,
            year,
            hour,
            minute,
            second: 0,
            millisecond: 0,
        })
    }
}

impl fmt::Display for Date {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "{:02}/{:02}/{} {:02}:{:02}",
            self.day, self.month, self.year, self.hour, self.minute
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_date_to_string() {
        let date = Date {
            day: 18,
            month: 4,
            year: 2026,
            hour: 12,
            minute: 30,
            second: 0,
            millisecond: 0,
        };
        assert_eq!(date.to_string(), "18/04/2026 12:30");
    }

    #[test]
    fn test_date_from_string() {
        let date_str = "18/04/2026 12:30";
        let date = Date::from_string(date_str).unwrap();
        assert_eq!(date.day, 18);
        assert_eq!(date.month, 4);
        assert_eq!(date.year, 2026);
        assert_eq!(date.hour, 12);
        assert_eq!(date.minute, 30);
    }
}
