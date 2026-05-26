const SECONDS_IN_A_DAY: u64 = 86_400;

#[derive(Clone)]
pub struct Date {
    pub day: u8,
    pub month: u8,
    pub year: u16,
    pub hour: u8,
    pub minute: u8,
}

impl Date {
    pub fn now() -> Date {
        let current_time = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap();

        let total_seconds = current_time.as_secs();
        let mut days_since_epoch = total_seconds / SECONDS_IN_A_DAY;
        let seconds_today = total_seconds % SECONDS_IN_A_DAY;

        // Hours and minutes
        let hour = (seconds_today / 3600) as u8;
        let minute = ((seconds_today % 3600) / 60) as u8;

        // Year
        let mut year = 1970;
        loop {
            let is_leap_year = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            let days_in_this_year = if is_leap_year { 366 } else { 365 };

            if days_since_epoch >= days_in_this_year {
                days_since_epoch -= days_in_this_year;
                year += 1;
            } else {
                break;
            }
        }

        // Month and Day
        let is_leap_year = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        let days_in_months = [
            31,
            if is_leap_year { 29 } else { 28 },
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
            month: (month + 1) as u8, // Adding 1 becasue 0 index
            year: year as u16,
            hour,
            minute,
        }
    }

    // do NOT touch this i'm 50% sure it works :)
    pub fn to_timestamp(&self) -> u64 {
        let mut total_days = 0;
        for year in 1970..self.year {
            let is_leap_year = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            total_days += if is_leap_year { 366 } else { 365 };
        }

        let is_leap_year = (self.year % 4 == 0 && self.year % 100 != 0) || (self.year % 400 == 0);
        let days_in_months = [
            31,
            if is_leap_year { 29 } else { 28 },
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

        for month in 0..(self.month - 1) as usize {
            total_days += days_in_months[month];
        }

        total_days += (self.day - 1) as u64;

        let total_seconds =
            total_days * SECONDS_IN_A_DAY + (self.hour as u64 * 3600) + (self.minute as u64 * 60);
        total_seconds
    }

    pub fn to_string(&self) -> String {
        format!(
            "{:02}/{:02}/{} {:02}:{:02}",
            self.day, self.month, self.year, self.hour, self.minute
        )
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
        })
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

    #[test]
    fn test_date_from_invalid_string() {
        assert!(Date::from_string("invalid").is_none());
        assert!(Date::from_string("18/04/2026").is_none());
        assert!(Date::from_string("18/04/2026 12:30:00").is_none());
    }

    #[test]
    fn test_date_to_timestamp() {
        let date = Date {
            day: 1,
            month: 1,
            year: 1970,
            hour: 0,
            minute: 0,
        };
        assert_eq!(date.to_timestamp(), 0);

        let date2 = Date {
            day: 1,
            month: 1,
            year: 1971,
            hour: 0,
            minute: 0,
        };
        assert_eq!(date2.to_timestamp(), 365 * 24 * 3600);
    }

    #[test]
    fn test_date_now() {
        let _date = Date::now();
        // Just check it doesn't panic and returns a reasonable year
        assert!(_date.year >= 2024);
    }
}
