<script lang="ts">
	interface FormData {
		name: string;
		email: string;
		message: string;
	}

	let formData: FormData = {
		name: '',
		email: '',
		message: ''
	};

	let submitted: boolean = false;
	let error: boolean = false;

	function handleSubmit(): void {
		if (formData.name && formData.email && formData.message) {
			submitted = true;
			error = false;
		} else {
			error = true;
		}
	}
</script>

<svelte:head>
	<title>Contact | BikeTitans</title>
</svelte:head>

<section>
	<h1 class="mb-6 text-4xl font-bold">Contact Us</h1>

	<div class="grid grid-cols-1 gap-8 md:grid-cols-2">
		<div class="rounded-lg bg-white p-8 shadow-md">
			<h2 class="mb-4 text-2xl font-bold">Get in Touch</h2>

			{#if submitted}
				<div class="mb-6 rounded border border-green-400 bg-green-100 px-4 py-3 text-green-700">
					Thank you for your message! We'll get back to you soon.
				</div>
			{:else}
				{#if error}
					<div class="mb-6 rounded border border-red-400 bg-red-100 px-4 py-3 text-red-700">
						Please fill in all fields.
					</div>
				{/if}

				<form on:submit|preventDefault={handleSubmit} class="space-y-6">
					<div>
						<label for="name" class="mb-2 block text-gray-700">Name</label>
						<input
							id="name"
							type="text"
							bind:value={formData.name}
							class="w-full rounded-md border px-4 py-2 focus:ring-2 focus:ring-blue-500 focus:outline-none"
						/>
					</div>

					<div>
						<label for="email" class="mb-2 block text-gray-700">Email</label>
						<input
							id="email"
							type="email"
							bind:value={formData.email}
							class="w-full rounded-md border px-4 py-2 focus:ring-2 focus:ring-blue-500 focus:outline-none"
						/>
					</div>

					<div>
						<label for="message" class="mb-2 block text-gray-700">Message</label>
						<textarea
							id="message"
							bind:value={formData.message}
							rows="5"
							class="w-full rounded-md border px-4 py-2 focus:ring-2 focus:ring-blue-500 focus:outline-none"
						></textarea>
					</div>

					<button
						type="submit"
						class="rounded-md bg-blue-600 px-6 py-2 text-white transition-colors hover:bg-blue-700"
					>
						Send Message
					</button>
				</form>
			{/if}
		</div>

		<div class="rounded-lg bg-white p-8 shadow-md">
			<h2 class="mb-4 text-2xl font-bold">Contact Information</h2>

			<div class="space-y-6">
				<div>
					<h3 class="mb-2 text-lg font-semibold">Address</h3>
					<p class="text-gray-700">
						123 Main Street<br />
						Suite 456<br />
						Anytown, USA 12345
					</p>
				</div>

				<div>
					<h3 class="mb-2 text-lg font-semibold">Email</h3>
					<p class="text-gray-700">info@BikeTitans.com</p>
				</div>

				<div>
					<h3 class="mb-2 text-lg font-semibold">Phone</h3>
					<p class="text-gray-700">(123) 456-7890</p>
				</div>

				<div>
					<h3 class="mb-2 text-lg font-semibold">Business Hours</h3>
					<p class="text-gray-700">
						Monday - Friday: 9am - 5pm<br />
						Saturday & Sunday: Closed
					</p>
				</div>
			</div>
		</div>
	</div>
</section>
