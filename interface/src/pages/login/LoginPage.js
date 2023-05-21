import './LoginPage.scss';

import { useState } from 'react';
import { Navigate } from 'react-router-dom';

import AuthService from '../../services/auth.service';

function LoginPage() {
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [isLoggedIn, setIsLoggedIn] = useState(false);

  const logInByEnter = (event) => {
    if (event.key === 'Enter') {
      logIn();
    }
  };

  const logIn = async () => {
    console.log('doing it now', { username, password });
    setIsLoggedIn(await AuthService.logIn(username, password));
  };

  if (isLoggedIn) {
    return <Navigate to='/' />;
  }

  return (
    <div className='login-page-container'>
      <div className='card login-card' onKeyDown={logInByEnter}>
        <div className='form-group'>
          <label className='form-label' htmlFor='username'>
            username
          </label>
          <input
            className='form-input'
            type='text'
            id='username'
            placeholder='username'
            onKeyUp={(event) => setUsername(event.target.value)}
          />
        </div>
        <div className='form-group'>
          <label className='form-label' htmlFor='password'>
            password
          </label>
          <input
            className='form-input'
            type='password'
            id='password'
            placeholder='password'
            onKeyUp={(event) => setPassword(event.target.value)}
          />
        </div>
        <button className='btn btn-primary btn-login' onClick={logIn}>
          Login
        </button>
      </div>
    </div>
  );
}

export default LoginPage;
