const fs = require('fs');
const crypto = require('crypto');

const TOKEN_CHARACTERS = 'qwertyuiop[]asdfghjklzxcvbnm,.1234567890!@#$%^&*()';
const TOKEN_LENGTH = 32;

let token = null;

/**
 * Ig the credentials are correct, returns a login token.
 * @param {string} username the username used for logging in
 * @param {string} password tha password used for logging in
 * @returns {string} return a token or null
 */
function logIn(username, password) {
  let users = [];
  token = Array(TOKEN_LENGTH)
    .fill(0)
    .map(() => TOKEN_CHARACTERS[crypto.randomInt(0, TOKEN_CHARACTERS.length)])
    .join('');

  try {
    users = JSON.parse(fs.readFileSync('db/users.json'));
  } catch (error) {
    console.error('Failed to load users', error);
  }

  if (
    users?.find(
      (user) => user.username === username && user.password === password
    )
  ) {
    return token;
  }
  return null;
}

/**
 * Returns true if the user with the given token is logged in
 * @param {string} tokenToCheck
 * @returns {boolean}
 */
function isLoggedIn(tokenToCheck) {
  return token === tokenToCheck;
}

module.exports = { logIn, isLoggedIn };
