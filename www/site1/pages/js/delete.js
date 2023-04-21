document.getElementById('title-form').addEventListener('submit', async function (event) {
	event.preventDefault();

	const baseURL = `${window.location.protocol}//${window.location.hostname}:${window.location.port}`;
	try {
		const filename = document.getElementById('form-name').value;
		if (filename == "." || filename == ".." || filename.includes("../")) {
			throw new Error("Invalid filename");
		}
		const response = await fetch(`${baseURL}/uploads/${filename}`, {
			method: 'DELETE'
		});
		if (response.status && response.status == 301) {
			alert("reloc");
			const redirectUrl = response.headers.get('Location');
			window.location.assign(redirectUrl);
		}
		else if (response.ok)
			alert('[OK] La ressource a été supprimée avec succès.');
		else
			alert("[ERROR] " + response.statusText);
	} catch (error) {
		alert("[ERROR] " + error);
	}
});