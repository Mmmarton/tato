import HTTPService from './http.service';

const AuthService = {
  /**
   * Attempts to log in the user
   * @param {string} token
   */
  logIn: async function logIn(username, password) {
    const token = await HTTPService.post('http://localhost:8080/login', {
      username,
      password,
    });
    if (!token || !token.token) {
      return null;
    }
    this.setToken(token.token);
    return true;
  },

  /**
   * Sets the current token.
   * @param {string} token
   */
  setToken: function setToken(token) {
    localStorage.setItem('token', token);
  },

  /**
   * Sets the current token.
   * @returns {string}
   */
  getToken: function getToken() {
    return localStorage.getItem('token');
  },
};

export default AuthService;
