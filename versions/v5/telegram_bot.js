
const TelegramBot = require('node-telegram-bot-api');

// Replace with your Telegram Bot Token
const token = 'YOUR_TELEGRAM_BOT_TOKEN';
const bot = new TelegramBot(token, { polling: true });

// Emergency alert function
function sendEmergencyAlert(chatId, message) {
  bot.sendMessage(chatId, message, { parse_mode: 'Markdown' });
}

// Example usage:
// sendEmergencyAlert('CHAT_ID', '*Emergency Alert!*\n\nUser: John Doe\nAge: 30\nCondition: Fall Detected\nLocation: [View on Map](https://www.google.com/maps?q=34.0522,-118.2437)');

module.exports = { sendEmergencyAlert };
