document.getElementById('sub-form-get').addEventListener('submit', async function (event) {
	event.preventDefault();

	try {
		const scriptName = document.getElementById('form-get-name').value;
		const scriptArgs = document.getElementById('form-get-argz').value;

		const baseURL = `${window.location.protocol}//${window.location.hostname}:${window.location.port}`;
		const response = await fetch(`${baseURL}/php/${scriptName}?${scriptArgs}`);
		if (response.ok) {
			const data = await response.text();
			document.getElementById("sub-form-get").innerHTML = `<p>${data}</p>`;
			alert("[OK] Réponse reçue");
		}
		else {
			alert("[ERROR] " + response.statusText);
		}
	} catch (error) {
		alert("[ERROR] " + error);
	}
});

document.getElementById('sub-form-post').addEventListener('submit', async function (event) {
	event.preventDefault();

	const baseURL = `${window.location.protocol}//${window.location.hostname}:${window.location.port}`;
	try {
		const postBody = new FormData();
		document.getElementById('form-post-argz').value.split('&').forEach(arg => {
			const [key, value] = arg.split('=');
			postBody.append(key, value);
		});

		const response = await fetch(`${baseURL}/php/${document.getElementById('form-post-name').value}`, {
			method: 'POST',
			headers: {
			  'Content-Type': 'application/x-www-form-urlencoded'
			},
			body: document.getElementById('form-post-argz').value
		});
		if (response.ok) {
			const data = await response.text();
			document.getElementById("sub-form-post").innerHTML = `<p>${data}</p>`;
			alert("[OK] Réponse reçue");
		}
		else
			alert("[ERROR] " + response.statusText);
	} catch (error) {
		alert("[ERROR] " + error);
	}
});
