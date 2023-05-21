import './LoginPage.scss';

function LoginPage() {
  return (
    <div className='login-page-container'>
      <div className='form-group'>
        <label className='form-label' htmlFor='username'>
          username
        </label>
        <input
          className='form-input'
          type='text'
          id='username'
          placeholder='username'
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
        />
      </div>
    </div>
  );
}

export default LoginPage;
