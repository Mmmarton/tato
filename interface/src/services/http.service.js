import { getToken } from './auth.service';

const HTTPService = {
  put: async (url, data) => {
    return fetch(url, {
      method: 'PUT',
      headers: {
        'Content-Type': 'application/json;charset=utf-8',
        Authorization: getToken(),
      },
      body: JSON.stringify(data),
    })
      .then((response) => {
        return response.json();
      })
      .catch(console.error);
  },

  post: async (url, data) => {
    return fetch(url, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json;charset=utf-8',
        Authorization: getToken(),
      },
      body: JSON.stringify(data),
    })
      .then((response) => {
        return response.json();
      })
      .catch(console.error);
  },

  get: async (url) => {
    return fetch(url, {
      method: 'GET',
      headers: {
        'Content-Type': 'application/json;charset=utf-8',
        Authorization: getToken(),
      },
    })
      .then((response) => {
        return response.json();
      })
      .catch(console.error);
  },

  delete: async (url) => {
    return fetch(url, {
      method: 'DELETE',
      headers: {
        'Content-Type': 'application/json;charset=utf-8',
        Authorization: getToken(),
      },
    })
      .then((response) => {
        return response.json();
      })
      .catch(console.error);
  },
};

export default HTTPService;
