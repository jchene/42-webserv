// Fomulaire par default
document.addEventListener('DOMContentLoaded', function () {
	document.getElementById('form-content').innerHTML = '<div id="form-content-div"><label for="form-content-name">Nom de la ressource :</label><input type="text" id="form-content-name"><label for="form-content-text">Contenu :</label><textarea id="form-content-text"></textarea></div>';
});

// Changement du formulaire
document.getElementById('form-type-text').addEventListener("click", function () {
	document.getElementById('form-content').innerHTML = '<div id="form-content-div"><label for="form-content-name">Nom de la ressource :</label><input type="text" id="form-content-name"><label for="form-content-text">Contenu :</label><textarea id="form-content-text"></textarea></div>';
	document.getElementById('form-type-file').checked = false;
});

document.getElementById('form-type-file').addEventListener("click", function () {
	document.getElementById('form-content').innerHTML = '<div id="form-content-div"><label for="form-content-file">Sélectionnez un fichier :</label><input type="file" id="form-content-file"></div>';
	document.getElementById('form-type-text').checked = false;
});

// Requête POST
document.getElementById('title-form').addEventListener('submit', async function (event) {
	event.preventDefault();

	const baseURL = `${window.location.protocol}//${window.location.hostname}:${window.location.port}`;
	try {
		const formData = new FormData();
		if (document.getElementById('form-type-text').checked)
		{
			const filename = `${document.getElementById("form-content-name").value}`;
			const content = `${document.getElementById("form-content-text").value}`;
			formData.set(filename, new File([content], filename, { type: 'text/plain' }));
		}
		else
			formData.append('file', document.getElementById('form-content-file').files[0]);

		const response = await fetch(`${baseURL}/uploads`, {
			method: 'POST',
			body: formData
		});
		if (response.ok)
			alert('[OK] Le fichier a été uploadé avec succès.');
		else
			alert('[ERROR] ' + response.statusText);
	} catch (error) {
		alert('[ERROR] ' + error);
	}
});