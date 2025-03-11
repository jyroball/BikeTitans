/** @type {import('@sveltejs/kit').Handle} */
export const handle = async ({ event, resolve }) => {
	const response = await resolve(event, {
		transformPageChunk: ({ html }) => html
	});

	return response;
};

// This sets trailing slashes globally
export const trailingSlash = 'always';
