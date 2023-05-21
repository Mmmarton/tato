/**
 * Sets the current token.
 * @param {string} token
 */
export function setToken(token) {
  localStorage.setItem('token', token);
}

/**
 * Sets the current token.
 * @returns {string}
 */
export function getToken() {
  return localStorage.getItem('token');
}
