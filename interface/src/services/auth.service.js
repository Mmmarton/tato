import HTTPService from './http.service';

const AuthService = {
  /**
   * Attempts to log in the user
   * @param {string} token
   */
  logIn: async function (username, password) {
    const token = await HTTPService.post('login', {
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
  setToken: function (token) {
    localStorage.setItem('token', token);
  },

  /**
   * Sets the current token.
   * @returns {string}
   */
  getToken: function () {
    return localStorage.getItem('token');
  },

  /**
   * Does a healthcheck call and returns false if the user is not logged in.
   * @returns {Promise<boolean>}
   */
  healthcheck: async function () {
    const response = await HTTPService.get('healthcheck');
    if (!response || !response.isLoggedIn) {
      if (this.getToken()) {
        this.setToken('');
        window.location.replace('/');
      }
    }
  },
};

export default AuthService;
