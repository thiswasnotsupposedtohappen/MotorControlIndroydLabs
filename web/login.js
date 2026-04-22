document.addEventListener('DOMContentLoaded', () => 
{
    const loginForm = document.getElementById('loginForm');
    const usernameInput = document.getElementById('username');
    const passwordInput = document.getElementById('password');
    const errorMsg = document.getElementById('loginError');

    // Hardcoded credentials
    const VALID_USERNAME = 'admin';
    const VALID_PASSWORD = 'password';

    loginForm.addEventListener('submit', (e) => 
    {
        e.preventDefault();
        
        const username = usernameInput.value;
        const password = passwordInput.value;

        if (username === VALID_USERNAME && password === VALID_PASSWORD) 
        {
            // Success
            errorMsg.classList.add('hidden');
            window.location.href = 'splash.html';
        } else 
        {
            // Failure
            errorMsg.classList.remove('hidden');
            passwordInput.value = '';
            
            // Add a shake effect to the container
            const container = document.querySelector('.login-container');
            container.classList.add('shake');
            setTimeout(() => container.classList.remove('shake'), 500);
        }
    });

    // Auto-focus username
    usernameInput.focus();
});
